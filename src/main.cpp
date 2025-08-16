#include <iostream>

#include "graphics/colors/colors.hpp"
#include "graphics/draw_info/draw_info.hpp"
#include "graphics/shader_standard/shader_standard.hpp"
#include "graphics/vertex_geometry/vertex_geometry.hpp"

// #include "string_invoker/string_invoker.hpp"

#include "meta_program/meta_program.hpp"
#include "system_logic/toolbox_engine/toolbox_engine.hpp"

#include "utility/glm_printing/glm_printing.hpp"
#include "utility/glm_utils/glm_utils.hpp"
#include "utility/meta_utils/meta_utils.hpp"
#include "utility/regex_utils/regex_utils.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

// TODO: I want to now do spdlog and get that printing out per iteration and then test that outout to generate input
// for invocations and see how that looks, then can start looking at frag.z

// TODO: next step is to create a program that helps you interactively call functions, how can we do this, well first we
// want to to select the function we want to call, through fuzzy or interactive way, once we've selected the function,
// we want to go through its parameters and ask the user to input the parameter, and by the end we have a potential
// function invocation, this will work fine, but the function invocation might not b evalidly created so having it
// intcrementally check if the parameters are corect would be best, and if its not correct then we prompt them to go
// again this would be optimal. also I want to handle default arguments better at some point

// i need a list of all the available meta functions, every string invoker file will have a vector

std::vector<std::string> load_invocations_from_file(const std::string &file_path) {
    std::vector<std::string> invocations;
    std::ifstream file(file_path);

    if (!file) {
        std::cerr << "Error: Unable to open file: " << file_path << std::endl;
        return invocations;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace if necessary
        if (!line.empty()) {
            invocations.push_back(line);
        }
    }

    return invocations;
}

struct Camera2D {
    float zoom = 1.0f;

    glm::vec2 offset = glm::vec2(0);

    double last_mouse_pos_x = 0.0f;
    double last_mouse_pos_y = 0.0f;

    bool is_dragging = false;

    glm::mat4 get_transform_matrix(int width, int height) {
        float aspect = static_cast<float>(width) / height;
        float view_width = zoom * aspect;
        float view_height = zoom;

        glm::mat4 projection = glm::ortho(-view_width + offset.x, view_width + offset.x, -view_height + offset.y,
                                          view_height + offset.y, -1.0f, 1.0f);
        return projection;
    }

    void on_scroll(double x_offset, double y_offset) {
        if (y_offset > 0) {
            zoom /= 1.1;
        } else {
            zoom *= 1.1;
        }
    }

    // TODO: one day I want to implement that "momentum style of dragging"
    void update(double mouse_delta_x, double mouse_delta_y, unsigned int width, unsigned int height, bool is_dragging) {
        if (is_dragging) {
            float aspect = static_cast<float>(width) / height;
            offset.x -= static_cast<float>(mouse_delta_x) / width * zoom * aspect * 2.0f;
            offset.y += static_cast<float>(mouse_delta_y) / height * zoom * 2.0f;
        }
    }
};

int main() {

    meta_utils::MetaType glm_vec3_type("glm::vec3", "[](const std::string &s) { return glm_utils::parse_vec3(s); }",
                                       "[](const glm::vec3 &v) { return vec3_to_string(v); }",
                                       regex_utils::float_triplet);
    meta_utils::MetaType glm_vec2_type("glm::vec2", "[](const std::string &s) { return glm_utils::parse_vec2(s); }",
                                       "[](const glm::vec2 &v) { return vec2_to_string(v); }",
                                       regex_utils::float_tuple);

    meta_utils::MetaType meta_type_type("meta_utils::MetaType", "", "", "MetaType");

    meta_utils::meta_types.add_new_concrete_type(glm_vec3_type);
    meta_utils::meta_types.add_new_concrete_type(glm_vec2_type);
    meta_utils::meta_types.add_new_concrete_type(meta_type_type);

    cpp_parsing::test();

    // TODO: we want to automate the meta type construction....
    meta_utils::MetaFunctionSignature ivp_constructor("IndexedVertexPositions(std::vector<unsigned int> indices, "
                                                      "std::vector<glm::vec3> xyz_positions, int id = "
                                                      "GlobalUIDGenerator::get_id())",
                                                      "");

    auto x = meta_utils::generate_string_invoker_for_function(ivp_constructor);

    meta_utils::MetaFunction mfx(x);

    meta_utils::MetaType ivp_type("draw_info::IndexedVertexPositions", mfx.to_lambda_string(),
                                  "[](const draw_info::IndexedVertexPositions &ivp) { std::ostringstream "
                                  "oss; oss << ivp; return oss.str(); } ",
                                  regex_utils::tuple_of({regex_utils::float_regex, regex_utils::float_triplet}));

    meta_utils::MetaFunctionSignature rectangle_constructor(
        "Rectangle(glm::vec3 center = glm::vec3(0), float width = 2, float height = 2)", "vertex_geometry");

    auto y = meta_utils::generate_string_invoker_for_function(rectangle_constructor);

    meta_utils::MetaFunction mfy(y);

    meta_utils::MetaType rectangle_type(
        "vertex_geometry::Rectangle", mfy.to_lambda_string(),
        "[](const vertex_geometry::Rectangle &rect) { std::ostringstream "
        "oss; oss << rect; return oss.str(); } ",
        "Rectangle" +
            regex_utils::tuple_of({regex_utils::float_triplet, regex_utils::float_regex, regex_utils::float_regex}));

    meta_utils::meta_types.add_new_concrete_type(ivp_type);
    meta_utils::meta_types.add_new_concrete_type(rectangle_type);

    // TODO: I am going to make a function that takes in a vector of the below settings, this is a program wide string
    // invoker function, which allows us to call a single invoke function of a certain type and call the function we
    // require it does so by passing it on to each of the other header/source invokers to see if it will run
    // successfully, ie not optional and then returns the result, allowing us to do "program wide stuff", which is
    // equivalent to like importing what we need.

    meta_utils::StringInvokerGenerationSettingsForHeaderSource vg_settings(
        "src/graphics/vertex_geometry/vertex_geometry.hpp", "src/graphics/vertex_geometry/vertex_geometry.cpp", false,
        true,
        {" draw_info::IndexedVertexPositions "
         "generate_rectangle_between_2d(const "
         "glm::vec2 &p1, const glm::vec2 &p2, float thickness) ",
         "draw_info::IndexedVertexPositions generate_circle(const glm::vec3 "
         "center, float radius, unsigned int "
         "num_sides)"},
        meta_utils::FilterMode::Whitelist);

    meta_utils::StringInvokerGenerationSettingsForHeaderSource gf_settings(
        "src/graphics/grid_font/grid_font.hpp", "src/graphics/grid_font/grid_font.cpp", false, true);

    meta_utils::generate_string_invokers_program_wide({vg_settings, gf_settings});

#ifdef GENERATED_META_PROGRAM
    meta_program::MetaProgram meta_program(meta_utils::meta_types.get_concrete_types());
#endif

    std::unordered_map<SoundType, std::string> sound_type_to_file = {
        {SoundType::UI_HOVER, "assets/sounds/hover.wav"}, {SoundType::UI_CLICK, "assets/sounds/click.wav"},
        // {SoundType::UI_SUCCESS, "assets/sounds/success.wav"},
    };

    Camera2D camera;

    ToolboxEngine tbx_engine(
        "tbx_draw", {ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX, ShaderType::TRANSFORM_V_WITH_COLORED_VERTEX},
        sound_type_to_file);

    tbx_engine.window.enable_fullscreen();

    std::function<void(double, double)> scroll_callback = [&](double x_offset, double y_offset) {
        camera.on_scroll(x_offset, y_offset);
    };

    tbx_engine.glfw_lambda_callback_manager.set_scroll_callback(scroll_callback);

    std::string file_path = "invocations.txt";
    std::vector<std::string> invocations = load_invocations_from_file(file_path);

    std::vector<draw_info::IVPColor> ivpcs;

#ifdef GENERATED_META_PROGRAM
    for (const auto &invocation : invocations) {
        // TODO: turn Ind..Vert.. Pos to snake case I don't like camel beinghere
        auto result_opt = meta_program.invoker_that_returns_draw_info_IndexedVertexPositions(invocation);
        if (result_opt) {
            const auto &ivp = result_opt.value();

            // You can assign a color based on index or content of the
            // invocation
            draw_info::IVPColor colored(ivp, colors::white); // replace color as needed

            colored.id = tbx_engine.batcher.transform_v_with_colored_vertex_shader_batcher.object_id_generator.get_id();

            ivpcs.push_back(colored);
        } else {
            std::cerr << "Failed to invoke: " << invocation << "\n";
        }
    }
#endif

    // TODO instead pass in the resolution?
    tbx_engine.shader_cache.set_uniform(ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX,
                                        ShaderUniformVariable::ASPECT_RATIO, glm::vec2(1, 1));

    auto tick = [&](double dt) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto [mouse_delta_x, mouse_delta_y] = tbx_engine.input_state.get_mouse_delta();
        camera.update(mouse_delta_x, mouse_delta_y, tbx_engine.window.width_px, tbx_engine.window.height_px,
                      tbx_engine.input_state.is_pressed(EKey::LEFT_MOUSE_BUTTON));

        tbx_engine.shader_cache.set_uniform(
            ShaderType::TRANSFORM_V_WITH_COLORED_VERTEX, ShaderUniformVariable::TRANSFORM,
            camera.get_transform_matrix(tbx_engine.window.width_px, tbx_engine.window.height_px));

        for (const auto &ivpc : ivpcs) {
            // tbx_engine.batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(ivpc);
            tbx_engine.batcher.transform_v_with_colored_vertex_shader_batcher.queue_draw(ivpc);
        }

        // tbx_engine.batcher.absolute_position_with_colored_vertex_shader_batcher.draw_everything();
        tbx_engine.batcher.transform_v_with_colored_vertex_shader_batcher.draw_everything();

        TemporalBinarySignal::process_all();

        glfwSwapBuffers(tbx_engine.window.glfw_window);
        glfwPollEvents();
    };
    auto term = [&]() { return tbx_engine.window_should_close(); };
    tbx_engine.main_loop.start(tick, term);

    return 0;
}
