#include <iostream>
#include "graphics/colors/colors.hpp"
#include "graphics/shader_standard/shader_standard.hpp"
#include "graphics/vertex_geometry/vertex_geometry.hpp"
#include "system_logic/toolbox_engine/toolbox_engine.hpp"

// https://chatgpt.com/c/68808a85-ca34-8007-8693-5dd3e7510abf
// continue working on custom vp file type

int main() {

    std::unordered_map<SoundType, std::string> sound_type_to_file = {
        {SoundType::UI_HOVER, "assets/sounds/hover.wav"}, {SoundType::UI_CLICK, "assets/sounds/click.wav"},
        // {SoundType::UI_SUCCESS, "assets/sounds/success.wav"},
    };

    ToolboxEngine tbx_engine("tbx_draw", {ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX}, sound_type_to_file);

    auto circle = vertex_geometry::generate_circle(glm::vec3(0), 0.1);
    auto x_axis = vertex_geometry::generate_rectangle_between_2d(glm::vec2(-1, -1), glm::vec2(1, -1), 0.1);

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
