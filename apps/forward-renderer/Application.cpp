#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        // Put here rendering code
        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto projMatrix = glm::perspective(70.f, float(viewportSize.x) / viewportSize.y, 0.01f, 100.f);
        const auto viewMatrix = m_viewController.getViewMatrix(); //glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        {
            const auto modelMatrix = glm::rotate(glm::translate(glm::mat4(1), glm::vec3(-2, 0, 0)), 0.2f * float(seconds), glm::vec3(0, 1, 0));

            const auto mvMatrix = viewMatrix * modelMatrix;
            const auto mvpMatrix = projMatrix * mvMatrix;
            const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));

            glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
            glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));

            glBindVertexArray(m_cubeVAO);
            glDrawElements(GL_TRIANGLES, m_cubeGeometry.indexBuffer.size(), GL_UNSIGNED_INT, nullptr);
        }

        {
            const auto modelMatrix = glm::rotate(glm::translate(glm::mat4(1), glm::vec3(2, 0, 0)), 0.2f * float(seconds), glm::vec3(0, 1, 0));

            const auto mvMatrix = viewMatrix * modelMatrix;
            const auto mvpMatrix = projMatrix * mvMatrix;
            const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));

            glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
            glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));

            glBindVertexArray(m_sphereVAO);
            glDrawElements(GL_TRIANGLES, m_sphereGeometry.indexBuffer.size(), GL_UNSIGNED_INT, nullptr);
        }

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

        
        ImGui::Render();

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        m_GLFWHandle.swapBuffers();

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            m_viewController.update(float(ellapsedTime));
        }
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" }

{
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

    const GLint vboBindingIndex = 0; // Arbitrary choice between 0 and glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS)

    const GLint positionAttrLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint texCoordsAttrLocation = 2;

    glCreateBuffers(1, &m_cubeVBO);
    glCreateBuffers(1, &m_cubeIBO);
    glCreateBuffers(1, &m_sphereVBO);
    glCreateBuffers(1, &m_sphereIBO);

    m_cubeGeometry = glmlv::makeCube();
    m_sphereGeometry = glmlv::makeSphere(32);

    glNamedBufferStorage(m_cubeVBO, m_cubeGeometry.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), m_cubeGeometry.vertexBuffer.data(), 0);
    glNamedBufferStorage(m_sphereVBO, m_sphereGeometry.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), m_sphereGeometry.vertexBuffer.data(), 0);

    glNamedBufferStorage(m_cubeIBO, m_cubeGeometry.indexBuffer.size() * sizeof(uint32_t), m_cubeGeometry.indexBuffer.data(), 0);
    glNamedBufferStorage(m_sphereIBO, m_sphereGeometry.indexBuffer.size() * sizeof(uint32_t), m_sphereGeometry.indexBuffer.data(), 0);

    // Cube
    glCreateVertexArrays(1, &m_cubeVAO);

    glVertexArrayVertexBuffer(m_cubeVAO, vboBindingIndex, m_cubeVBO, 0, sizeof(glmlv::Vertex3f3f2f));

    glVertexArrayAttribBinding(m_cubeVAO, positionAttrLocation, vboBindingIndex);
    glEnableVertexArrayAttrib(m_cubeVAO, positionAttrLocation);
    glVertexArrayAttribFormat(m_cubeVAO, positionAttrLocation, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, position));

    glVertexArrayAttribBinding(m_cubeVAO, normalAttrLocation, vboBindingIndex);
    glEnableVertexArrayAttrib(m_cubeVAO, normalAttrLocation);
    glVertexArrayAttribFormat(m_cubeVAO, normalAttrLocation, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, normal));

    glVertexArrayAttribBinding(m_cubeVAO, texCoordsAttrLocation, vboBindingIndex);
    glEnableVertexArrayAttrib(m_cubeVAO, texCoordsAttrLocation);
    glVertexArrayAttribFormat(m_cubeVAO, texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, texCoords));

    glVertexArrayElementBuffer(m_cubeVAO, m_cubeIBO);

    // Sphere
    glCreateVertexArrays(1, &m_sphereVAO);

    glVertexArrayVertexBuffer(m_sphereVAO, vboBindingIndex, m_sphereVBO, 0, sizeof(glmlv::Vertex3f3f2f));

    glVertexArrayAttribBinding(m_sphereVAO, positionAttrLocation, vboBindingIndex);
    glEnableVertexArrayAttrib(m_sphereVAO, positionAttrLocation);
    glVertexArrayAttribFormat(m_sphereVAO, positionAttrLocation, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, position));

    glVertexArrayAttribBinding(m_sphereVAO, normalAttrLocation, vboBindingIndex);
    glEnableVertexArrayAttrib(m_sphereVAO, normalAttrLocation);
    glVertexArrayAttribFormat(m_sphereVAO, normalAttrLocation, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, normal));

    glVertexArrayAttribBinding(m_sphereVAO, texCoordsAttrLocation, vboBindingIndex);
    glEnableVertexArrayAttrib(m_sphereVAO, texCoordsAttrLocation);
    glVertexArrayAttribFormat(m_sphereVAO, texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, texCoords));

    glVertexArrayElementBuffer(m_sphereVAO, m_sphereIBO);

    glEnable(GL_DEPTH_TEST);

    m_program = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "forward.vs.glsl", m_ShadersRootPath / m_AppName / "forward.fs.glsl" });
    m_program.use();

    m_uModelViewProjMatrixLocation = glGetUniformLocation(m_program.glId(), "uModelViewProjMatrix");
    m_uModelViewMatrixLocation = glGetUniformLocation(m_program.glId(), "uModelViewMatrix");
    m_uNormalMatrixLocation = glGetUniformLocation(m_program.glId(), "uNormalMatrix");
}