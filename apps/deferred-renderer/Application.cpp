#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>

using namespace glm;
using namespace glmlv;

int Application::run()
{
    vec3 clearColor(0, 186/255.f, 1.f);
    glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);

    const float maxCameraSpeed = m_Scene.getDiagonalLength() * 0.5f;
    float cameraSpeed = maxCameraSpeed / 5.f;
    m_ViewController.setSpeed(cameraSpeed);

    SceneInstanceData sceneInstance;
    sceneInstance.modelMatrix = translate(mat4(1), vec3(2,0,-2));

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render everything
        m_DeferredGPassProgram.use();
        m_DeferredGPassProgram.resetMaterialUniforms();
        static bool hasWarned = false;
        if(!hasWarned) {
            hasWarned = true;
            std::cout << "NOTE: We're not rendering anything right now!" << std::endl;
        }
        // m_Scene.render(m_DeferredGPassProgram, m_ViewController, sceneInstance); // FIXME

        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", &clearColor[0])) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }

            if(ImGui::SliderFloat("Camera speed", &cameraSpeed, 0.001f, maxCameraSpeed)) {
                m_ViewController.setSpeed(cameraSpeed);
            }
            ImGui::SliderFloat("near", &m_ViewController.m_Near, 0.0001f, 1.f);
            ImGui::SliderFloat("far", &m_ViewController.m_Far, 100.f, 10000.f);
            ImGui::End();
        }

        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);
        ImGui::Render();

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        m_GLFWHandle.swapBuffers();

        auto elapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            m_ViewController.update(float(elapsedTime));
        }
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_DeferredGPassProgram(
        m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl",
        m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl"
    ),
    m_Scene(m_AssetsRootPath / "glmlv" / "models" / "crytek-sponza" / "sponza.obj"),
    m_ViewController(m_GLFWHandle.window(), m_nWindowWidth, m_nWindowHeight),
    m_GBufferTextures {
        { m_GBufferTextureFormat[0], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight },
        { m_GBufferTextureFormat[1], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight },
        { m_GBufferTextureFormat[2], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight },
        { m_GBufferTextureFormat[3], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight },
        { m_GBufferTextureFormat[4], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight },
        { m_GBufferTextureFormat[5], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight }
    }
{
    (void) argc;
    static_ImGuiIniFilename = m_AppName + ".imgui.ini";
    ImGui::GetIO().IniFilename = static_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file
    glEnable(GL_DEPTH_TEST);
}

std::string Application::static_ImGuiIniFilename;
