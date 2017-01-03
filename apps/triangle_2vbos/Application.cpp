#include "Application.hpp"

#include <iostream>

#include <glm/glm.hpp>
#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);

        // Put here rendering code
        glBindVertexArray(m_triangleVAO);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(0);

        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", clearColor)) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }
            ImGui::End();
        }

        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);
        ImGui::Render();

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        m_GLFWHandle.swapBuffers();

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            //viewController.update(float(ellapsedTime))
        }
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath{ glmlv::fs::path{ argv[0] } },
    m_AppName{ m_AppPath.stem().string() },
    m_ImGuiIniFilename{ m_AppName + ".imgui.ini" },
    m_ShadersRootPath{ m_AppPath.parent_path() / "shaders" }
{
    glCreateBuffers(1, &m_trianglePositionsVBO);

    glm::vec2 trianglePositions[] = {
        glm::vec2(-0.5, -0.5),
        glm::vec2(0.5, -0.5),
        glm::vec2(0, 0.5),
    };

    glNamedBufferStorage(m_trianglePositionsVBO, sizeof(trianglePositions), trianglePositions, 0);

    glCreateBuffers(1, &m_triangleColorsVBO);

    glm::vec3 triangleColors[] = {
        glm::vec3(1, 0, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, 0, 1)
    };

    glNamedBufferStorage(m_triangleColorsVBO, sizeof(triangleColors), triangleColors, 0);

    glCreateVertexArrays(1, &m_triangleVAO);

    const GLint positionVBOBindingIndex = 0; // Arbitrary choice between 0 and glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS)
    const GLint colorVBOBindingIndex = 1; // Arbitrary choice between 0 and glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS)

    // Vertex attrib locations are defined in the vertex shader (we can also use glGetAttribLocation(program, attribname) with attribute names after program compilation in order to get these numbers)
    const GLint positionAttrLocation = 4;
    const GLint colorAttrLocation = 2;

    glVertexArrayVertexBuffer(m_triangleVAO, positionVBOBindingIndex, m_trianglePositionsVBO, 0, sizeof(glm::vec2));
    glVertexArrayVertexBuffer(m_triangleVAO, colorVBOBindingIndex, m_triangleColorsVBO, 0, sizeof(glm::vec3));

    glVertexArrayAttribBinding(m_triangleVAO, positionAttrLocation, positionVBOBindingIndex);
    glEnableVertexArrayAttrib(m_triangleVAO, positionAttrLocation);
    glVertexArrayAttribFormat(m_triangleVAO, positionAttrLocation, 2, GL_FLOAT, GL_FALSE, 0);

    glVertexArrayAttribBinding(m_triangleVAO, colorAttrLocation, colorVBOBindingIndex);
    glEnableVertexArrayAttrib(m_triangleVAO, colorAttrLocation);
    glVertexArrayAttribFormat(m_triangleVAO, colorAttrLocation, 3, GL_FLOAT, GL_FALSE, 0);

    m_program = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "triangle.vs.glsl", m_ShadersRootPath / m_AppName / "triangle.fs.glsl" });
    m_program.use();
}

Application::~Application()
{
    if (m_trianglePositionsVBO) {
        glDeleteBuffers(1, &m_trianglePositionsVBO);
    }

    if (m_triangleColorsVBO) {
        glDeleteBuffers(1, &m_triangleColorsVBO);
    }

    if (m_triangleVAO) {
        glDeleteBuffers(1, &m_triangleVAO);
    }

    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();
}