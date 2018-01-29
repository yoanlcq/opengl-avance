#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_ForwardProgram.use();

        using namespace glm;

        const float fovy = radians(60.f), near = 0.001, far = 1000, aspect = m_nWindowWidth / float(m_nWindowHeight);
        mat4 proj(perspective(fovy, aspect, near, far));
        mat4 model(translate(mat4(1), vec3(0,0,-2)));
        //mat4 view(lookAt(vec3(0,0,0), vec3(0,0,1), vec3(0,1,0)));
        mat4 view(1);
        mat4 modelView(view * model);
        mat4 modelViewProj(proj * modelView);
        mat4 normalMatrix(transpose(inverse(modelView)));
        glUniformMatrix4fv(m_UniformModelViewProjMatrixLocation, 1, GL_FALSE, &modelViewProj[0][0]);
        glUniformMatrix4fv(m_UniformModelViewMatrixLocation, 1, GL_FALSE, &modelView[0][0]);
        glUniformMatrix4fv(m_UniformNormalMatrixLocation, 1, GL_FALSE, &normalMatrix[0][0]);

        //m_Cube.render();
        m_Sphere.render();

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
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_ForwardVsPath { m_ShadersRootPath / m_AppName / "forward.vs.glsl" },
    m_ForwardFsPath { m_ShadersRootPath / m_AppName / "forward.fs.glsl" },
    m_ForwardProgram(glmlv::compileProgram({ m_ForwardVsPath.string(), m_ForwardFsPath.string() })),
    m_UniformModelViewProjMatrixLocation(glGetUniformLocation(m_ForwardProgram.glId(), "uModelViewProjMatrix")),
    m_UniformModelViewMatrixLocation(glGetUniformLocation(m_ForwardProgram.glId(), "uModelViewMatrix")),
    m_UniformNormalMatrixLocation(glGetUniformLocation(m_ForwardProgram.glId(), "uNormalMatrix")),
    m_Cube(glmlv::makeCube()),
    m_Sphere(glmlv::makeSphere(32))
{
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

#define CHECK_UNIFORM(u) if(u == -1) { std::cerr << "Warning: " << #u << "equals -1 (is it unused?)" << std::endl; }
    CHECK_UNIFORM(m_UniformModelViewProjMatrixLocation != -1);
    CHECK_UNIFORM(m_UniformModelViewMatrixLocation != -1);
    CHECK_UNIFORM(m_UniformNormalMatrixLocation != -1);
#undef CHECK_UNIFORM

    glEnable(GL_DEPTH_TEST);
}
