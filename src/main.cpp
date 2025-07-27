#include <iostream>

#include "graphics/colors/colors.hpp"
#include "graphics/shader_standard/shader_standard.hpp"
#include "graphics/vertex_geometry/vertex_geometry.hpp"

#include "system_logic/toolbox_engine/toolbox_engine.hpp"

#include "utility/meta_utils/meta_types.hpp"
#include "utility/glm_utils/glm_utils.hpp"
#include "utility/glm_printing/glm_printing.hpp"
#include "utility/meta_utils/meta_utils.hpp"

// https://chatgpt.com/c/68808a85-ca34-8007-8693-5dd3e7510abf
// continue working on custom vp file type

int add(int x, int y) { return x + y; }
int mul(int x, int y) { return x * y; }
glm::vec3 mul_vec(glm::vec3 v, float c) { return v * c; }

float sum(std::vector<float> nums) {
    float sum = 0;
    for (const auto &n : nums) {
        sum += n;
    }
    return sum;
}

glm::vec3 sum_vec(std::vector<glm::vec3> vecs) {
    glm::vec3 sum(0.0f);
    for (const auto &v : vecs) {
        sum += v;
    }
    return sum;
}

std::optional<std::string> f(const std::string &input) {
    std::regex re(R"(^\s*add\s*\(\s*(-?\d+)\s*,\s*(-?\d+)\s*\)\s*$)");
    std::smatch match;
    if (!std::regex_match(input, match, re))
        return std::nullopt;

    int x = std::stoi(match[1]);
    int y = std::stoi(match[2]);

    int result = add(x, y);
    return std::to_string(result);
}

std::optional<std::string> g(const std::string &input) {
    std::regex re(
        R"(^\s*mul_vec\s*\(\s*(\s*\(\s*-?\d+(?:\.\d+)?\s*,\s*-?\d+(?:\.\d+)?\s*,\s*-?\d+(?:\.\d+)?\s*\)\s*)\s*,\s*(-?\d+(?:\.\d+)?)\s*\)\s*$)");
    std::smatch match;
    if (!std::regex_match(input, match, re))
        return std::nullopt;

    glm::vec3 v = glm_utils::parse_vec3(match[1]);
    float c = std::stod(match[2]);

    glm::vec3 result = mul_vec(v, c);
    return vec3_to_string(result);
}

std::optional<std::string> h(const std::string &input) {
    std::regex re(R"(^\s*sum\s*\(\s*(.*)\s*\)\s*$)");
    std::smatch match;
    if (!std::regex_match(input, match, re))
        return std::nullopt;

    auto conversion1 = [=](const std::string &input) -> std::vector<float> {
        std::string trimmed = input;

        // Remove surrounding curly braces if present
        if (!trimmed.empty() && trimmed.front() == '{' && trimmed.back() == '}') {
            trimmed = trimmed.substr(1, trimmed.size() - 2);
        }

        std::vector<float> result;
        std::stringstream ss(trimmed);
        std::string element;

        while (std::getline(ss, element, ',')) {
            // Trim whitespace
            element.erase(std::remove_if(element.begin(), element.end(), ::isspace), element.end());

            if (!element.empty()) {
                try {
                    auto conversion = [](const std::string &s) { return std::stof(s); };
                    auto converted = conversion(element);
                    result.push_back(converted);
                } catch (...) {
                    // Handle parse error if needed
                }
            }
        }

        return result;
    };
    std::vector<float> nums = conversion1(match[1]);

    std::cout << "testing" << std::endl;
    for (const auto &n : nums) {
        std::cout << n << std::endl;
    }

    int result = sum(nums);
    auto conversion = [](const int &v) { return std::to_string(v); };
    return conversion(result);
}

std::optional<std::string> i(const std::string &input) {
    std::regex re(R"(^\s*sum_vec\s*\(\s*(.*)\s*\)\s*$)");
    std::smatch match;
    if (!std::regex_match(input, match, re))
        return std::nullopt;

    auto conversion1 = [=](const std::string &input) -> std::vector<glm::vec3> {
        std::string trimmed = input;

        // Remove surrounding curly braces if present
        if (!trimmed.empty() && trimmed.front() == '{' && trimmed.back() == '}') {
            trimmed = trimmed.substr(1, trimmed.size() - 2);
        }

        std::vector<glm::vec3> result;
        std::regex element_re(R"(\s*\(\s*-?\d+(?:\.\d+)?\s*,\s*-?\d+(?:\.\d+)?\s*,\s*-?\d+(?:\.\d+)?\s*\)\s*)");
        auto begin = std::sregex_iterator(trimmed.begin(), trimmed.end(), element_re);
        auto end = std::sregex_iterator();

        for (auto it = begin; it != end; ++it) {
            try {
                auto conversion = [](const std::string &s) { return glm_utils::parse_vec3(s); };
                result.push_back(conversion(it->str()));
            } catch (...) {
                // Ignore malformed elements
            }
        }

        return result;
    };
    std::vector<glm::vec3> vecs = conversion1(match[1]);

    glm::vec3 result = sum_vec(vecs);
    auto conversion = [](const glm::vec3 &v) { return vec3_to_string(v); };
    return conversion(result);
}

std::optional<std::string> invoke(const std::string &invocation, std::vector<MetaType> available_types) {

    auto type_name_to_meta_type_map = meta_utils::create_type_name_to_meta_type_map(available_types);
    meta_utils::MetaFunctionSignature mf("int add(int x, int y)", type_name_to_meta_type_map);
    meta_utils::MetaFunctionSignature mg("glm::vec3 mul_vec(glm::vec3 v, float c)", type_name_to_meta_type_map);

    if (std::regex_match(invocation, std::regex(mf.invocation_regex))) {
        return f(invocation);
    } else if (std::regex_match(invocation, std::regex(mg.invocation_regex))) {
        return g(invocation);
    }

    return "No matching function signature.";
}

int main() {

    std::vector<MetaType> extended_types = meta_utils::concrete_types;
    MetaType glm_vec3_type("glm::vec3", "[](const std::string &s) { return glm_utils::parse_vec3(s); }",
                           "[](const glm::vec3 &v) { return vec3_to_string(v); }", regex_utils::float_triplet);
    MetaType glm_vec2_type("glm::vec2", "[](const std::string &s) { return glm_utils::parse_vec2(s); }",
                           "[](const glm::vec2 &v) { return vec2_to_string(v); }", regex_utils::float_tuple);
    extended_types.push_back(glm_vec3_type);
    extended_types.push_back(glm_vec2_type);

    MetaType vector_of_strings_type = meta_utils::parse_meta_type_from_string("std::vector<float>").value();
    std::cout << vector_of_strings_type.to_string() << std::endl;
    std::cout << meta_utils::generate_invoker("int add(int x, int y)", extended_types) << std::endl;
    std::cout << meta_utils::generate_invoker("glm::vec3 mul_vec(glm::vec3 v, float c)", extended_types) << std::endl;
    std::cout << meta_utils::generate_invoker("float sum(std::vector<float> nums)", extended_types) << std::endl;
    std::cout << meta_utils::generate_invoker("glm::vec3 sum_vec(std::vector<glm::vec3> vecs)", extended_types)
              << std::endl;

    std::cout << f("add(8, 3)").value() << std::endl;
    std::cout << g("mul_vec((1, 2, 3), 2)").value() << std::endl;
    std::cout << h("sum({3.2, 1.8, 1})").value() << std::endl;
    std::cout << i("sum_vec({(3.2, 1.8, 1), (1.8, 1, 3.2), (1, 3.2, 1.8)})").value() << std::endl;

    std::cout << invoke("mul_vec((1, 2, 3), 3)", extended_types).value() << std::endl;

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
