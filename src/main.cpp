#include <iostream>

#include "graphics/colors/colors.hpp"
#include "graphics/shader_standard/shader_standard.hpp"
#include "graphics/vertex_geometry/vertex_geometry.hpp"
#include "graphics/vertex_geometry/string_invoker/vertex_geometry.hpp"

#include "system_logic/toolbox_engine/toolbox_engine.hpp"

#include "utility/glm_printing/glm_printing.hpp"
#include "utility/glm_utils/glm_utils.hpp"
#include "utility/meta_utils/meta_utils.hpp"

void configure_meta_types() {}

int main() {

    std::vector<meta_utils::MetaType> extended_concrete_types = meta_utils::concrete_types;
    meta_utils::MetaType glm_vec3_type("glm::vec3", "[](const std::string &s) { return glm_utils::parse_vec3(s); }",
                                       "[](const glm::vec3 &v) { return vec3_to_string(v); }",
                                       regex_utils::float_triplet);
    meta_utils::MetaType glm_vec2_type("glm::vec2", "[](const std::string &s) { return glm_utils::parse_vec2(s); }",
                                       "[](const glm::vec2 &v) { return vec2_to_string(v); }",
                                       regex_utils::float_tuple);

    meta_utils::MetaType meta_type_type("meta_utils::MetaType", "", "", "MetaType");

    extended_concrete_types.push_back(glm_vec3_type);
    extended_concrete_types.push_back(glm_vec2_type);
    extended_concrete_types.push_back(meta_type_type);

    std::unordered_map<std::string, std::function<meta_utils::MetaType(meta_utils::MetaType)>>
        extended_generic_type_to_meta_type_constructor = meta_utils::generic_type_to_metatype_constructor;

    meta_utils::MetaFunctionSignature ivp_constructor(
        "IndexedVertexPositions(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions, int id = "
        "GlobalUIDGenerator::get_id())",
        "", meta_utils::create_type_name_to_meta_type_map(extended_concrete_types));

    auto x = meta_utils::generate_string_invoker_for_function(ivp_constructor, extended_concrete_types);

    meta_utils::MetaFunction mfx(x, meta_utils::create_type_name_to_meta_type_map(extended_concrete_types));

    meta_utils::MetaType ivp_type(
        "draw_info::IndexedVertexPositions", mfx.to_lambda_string(),
        "[](const draw_info::IndexedVertexPositions &ivp) { std::ostringstream oss; oss << ivp; return oss.str(); } ",
        regex_utils::tuple_of({regex_utils::float_regex, regex_utils::float_triplet}));

    extended_concrete_types.push_back(ivp_type);

    meta_utils::generate_string_invokers_from_source_code(
        "src/graphics/vertex_geometry/vertex_geometry.hpp", "src/graphics/vertex_geometry/vertex_geometry.cpp",
        extended_concrete_types, false, true,
        {" draw_info::IndexedVertexPositions generate_rectangle_between_2d(const "
         "glm::vec2 &p1, const glm::vec2 &p2, float thickness) ",
         "draw_info::IndexedVertexPositions generate_circle(const glm::vec3 center, float radius, unsigned int "
         "num_sides)"},
        meta_utils::FilterMode::Whitelist);

    std::unordered_map<SoundType, std::string> sound_type_to_file = {
        {SoundType::UI_HOVER, "assets/sounds/hover.wav"}, {SoundType::UI_CLICK, "assets/sounds/click.wav"},
        // {SoundType::UI_SUCCESS, "assets/sounds/success.wav"},
    };

    ToolboxEngine tbx_engine("tbx_draw", {ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX}, sound_type_to_file);

    auto invocation = "generate_rectangle_between_2d( (0, 0), (-1, -1), 0.01 )";
    auto invocation_circ = "generate_circle( (0, 0, 0), 0.2, 8 )";

    auto result = invoker_that_returns_draw_info_IndexedVertexPositions(invocation, extended_concrete_types);
    auto result_circ = invoker_that_returns_draw_info_IndexedVertexPositions(invocation_circ, extended_concrete_types);

    auto circle = result_circ.value();
    auto x_axis = result.value();

    auto colored_circle = draw_info::IVPColor(circle, colors::orangered2);
    auto colored_x_axi = draw_info::IVPColor(x_axis, colors::white);

    // TODO instead pass in the resolution?
    tbx_engine.shader_cache.set_uniform(ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX,
                                        ShaderUniformVariable::ASPECT_RATIO, glm::vec2(1, 1));

    auto tick = [&](double dt) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        tbx_engine.batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(colored_circle);
        tbx_engine.batcher.absolute_position_with_colored_vertex_shader_batcher.queue_draw(colored_x_axi);
        tbx_engine.batcher.absolute_position_with_colored_vertex_shader_batcher.draw_everything();

        glfwSwapBuffers(tbx_engine.window.glfw_window);
        glfwPollEvents();
    };
    auto term = []() { return false; };
    tbx_engine.main_loop.start(tick, term);

    return 0;
}
