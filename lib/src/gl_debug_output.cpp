#include <glmlv/gl_debug_output.hpp>
#include <glad/glad.h>
#include <array>
#include <tuple>
#include <iostream>
#include <imgui.h>

namespace glmlv
{

static std::array<std::tuple<const char *, bool, GLenum>, 6> sourceSelector =
{
    std::make_tuple("API", true, GL_DEBUG_SOURCE_API),
    std::make_tuple("WINDOW_SYSTEM", true, GL_DEBUG_SOURCE_WINDOW_SYSTEM),
    std::make_tuple("SHADER_COMPILER", true, GL_DEBUG_SOURCE_SHADER_COMPILER),
    std::make_tuple("THIRD_PARTY", true, GL_DEBUG_SOURCE_THIRD_PARTY),
    std::make_tuple("APPLICATION", true, GL_DEBUG_SOURCE_APPLICATION),
    std::make_tuple("OTHER", true, GL_DEBUG_SOURCE_OTHER),
};

static std::array<std::tuple<const char *, bool, GLenum>, 6> typeSelector =
{
    std::make_tuple("ERROR", true, GL_DEBUG_TYPE_ERROR),
    std::make_tuple("DEPRECATED_BEHAVIOR", true, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR),
    std::make_tuple("UNDEFINED_BEHAVIOR", true, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR),
    std::make_tuple("PORTABILITY", true, GL_DEBUG_TYPE_PORTABILITY),
    std::make_tuple("PERFORMANCE", true, GL_DEBUG_TYPE_PERFORMANCE),
    std::make_tuple("OTHER", true, GL_DEBUG_TYPE_OTHER),
};

static std::array<std::tuple<const char *, bool, GLenum>, 4> severitySelector =
{
    std::make_tuple("HIGH", true, GL_DEBUG_SEVERITY_HIGH),
    std::make_tuple("MEDIUM", true, GL_DEBUG_SEVERITY_MEDIUM),
    std::make_tuple("LOW", true, GL_DEBUG_SEVERITY_LOW),
    std::make_tuple("NOTIFICATION", false, GL_DEBUG_SEVERITY_NOTIFICATION)
};

void logGLDebugInfo(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, GLvoid* userParam);

void initGLDebugOutput()
{
    glDebugMessageCallback((GLDEBUGPROCARB)logGLDebugInfo, nullptr);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    for (auto & t : sourceSelector) {
        glDebugMessageControl(std::get<2>(t), GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, std::get<1>(t));
    }

    for (auto & t : typeSelector) {
        glDebugMessageControl(GL_DONT_CARE, std::get<2>(t), GL_DONT_CARE, 0, nullptr, std::get<1>(t));
    }

    for (auto & t : severitySelector) {
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, std::get<2>(t), 0, nullptr, std::get<1>(t));
    }
}

void showGLDebugOutputsWindow()
{
    ImGui::Begin("OpenGL Debug Outputs");

    if (ImGui::CollapsingHeader("Source"))
    {
        for (auto & t : sourceSelector) {
            if (ImGui::Checkbox(std::get<0>(t), &std::get<1>(t))) {
                glDebugMessageControl(std::get<2>(t), GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, std::get<1>(t));
            }
        }
    }

    if (ImGui::CollapsingHeader("Type"))
    {
        for (auto & t : typeSelector) {
            if (ImGui::Checkbox(std::get<0>(t), &std::get<1>(t))) {
                glDebugMessageControl(GL_DONT_CARE, std::get<2>(t), GL_DONT_CARE, 0, nullptr, std::get<1>(t));
            }
        }
    }

    if (ImGui::CollapsingHeader("Severity"))
    {
        for (auto & t : severitySelector) {
            if (ImGui::Checkbox(std::get<0>(t), &std::get<1>(t))) {
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, std::get<2>(t), 0, nullptr, std::get<1>(t));
            }
        }
    }

    ImGui::End();
}

void logGLDebugInfo(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, GLvoid* userParam)
{
    const auto findStr = [&](GLenum value, const auto & selector)
    {
        const auto it = find_if(begin(sourceSelector), end(sourceSelector), [source](const auto & t) { return std::get<2>(t) == source; });
        if (it == end(sourceSelector)) {
            return "UNDEFINED";
        }
        return std::get<0>(*it);
    };

    const auto sourceStr = findStr(source, sourceSelector);
    const auto typeStr = findStr(type, typeSelector);
    const auto severityStr = findStr(severity, severitySelector);

    std::clog << "OpenGL: " << message << " [source=" << sourceStr << " type=" << typeStr << " severity=" << severityStr << " id=" << id << "]\n\n";
}

}