#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>

using namespace glm;

int Application::run()
{
    vec3 clearColor(0,0,0);

    GLForwardRenderingProgram::SharedUniformData sharedUniformData;
    sharedUniformData.dirLightDir = vec3(1,0,0);
    sharedUniformData.dirLightIntensity = vec3(1,1,1);
    sharedUniformData.pointLightPosition = vec3(0,0,1);
    sharedUniformData.pointLightIntensity = vec3(1,1,1);

    m_ViewController.setSpeed(m_Scene.getDiagonalLength() * 0.1f); // 10% de la scene parcouru par seconde

    const GLuint sphereTextureUnit = 0;
    const GLuint cubeTextureUnit = 1;

    MeshInstanceData sphereInstance;
    sphereInstance.modelMatrix = translate(mat4(1), vec3(0,0,-2));
    sphereInstance.color = vec3(1,0,0);
    sphereInstance.textureUnit = sphereTextureUnit;

    MeshInstanceData cubeInstance;
    cubeInstance.modelMatrix = translate(mat4(1), vec3(-2,0,-2));
    cubeInstance.color = vec3(0,1,0);
    cubeInstance.textureUnit = cubeTextureUnit;

    SceneInstanceData sceneInstance;
    sceneInstance.modelMatrix = translate(mat4(1), vec3(2,0,-2));

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Do it every time because m_Scene overrides it when rendering
        glActiveTexture(GL_TEXTURE0 + sphereTextureUnit);
        m_SphereTex.bind();
        m_Sampler.bindToTextureUnit(sphereTextureUnit);
        glActiveTexture(GL_TEXTURE0 + cubeTextureUnit);
        m_CubeTex.bind();
        m_Sampler.bindToTextureUnit(cubeTextureUnit);


        m_ForwardProgram.use();
        m_ForwardProgram.setSharedUniformData(sharedUniformData);
        m_Sphere.render(m_ForwardProgram, m_ViewController, sphereInstance);
        m_Cube.render(m_ForwardProgram, m_ViewController, cubeInstance);
        m_Scene.render(m_ForwardProgram, m_ViewController, sceneInstance);

        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", &clearColor[0])) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }
#define EDIT_DIRECTION(c) ImGui::SliderFloat3(#c, &c[0], -1.f, 1.f)
            EDIT_DIRECTION(sharedUniformData.dirLightDir);
            EDIT_DIRECTION(sharedUniformData.pointLightPosition);
#undef EDIT_DIRECTION
#define EDIT_COLOR(c) ImGui::ColorEdit3(#c, &c[0])
            EDIT_COLOR(sharedUniformData.dirLightIntensity);
            EDIT_COLOR(sharedUniformData.pointLightIntensity);
            EDIT_COLOR(cubeInstance.color);
            EDIT_COLOR(sphereInstance.color);
#undef EDIT_COLOR
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
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_ForwardVsPath { m_ShadersRootPath / m_AppName / "forward.vs.glsl" },
    m_ForwardFsPath { m_ShadersRootPath / m_AppName / "forward.fs.glsl" },
    m_ForwardProgram(m_ForwardVsPath, m_ForwardFsPath),
    m_Sampler(GLSamplerParams().withWrapST(GL_REPEAT).withMinMagFilter(GL_LINEAR)),
    // Let's pretend the below two are different files for now (the code was planned for two files anyway)
    m_CubeTex  (m_AssetsRootPath / m_AppName / "textures" / "plasma.png"),
    m_SphereTex(m_AssetsRootPath / m_AppName / "textures" / "plasma.png"),
    m_Cube(glmlv::makeCube()),
    m_Sphere(glmlv::makeSphere(32)),
    m_Scene(m_AssetsRootPath / "glmlv" / "models" / "crytek-sponza" / "sponza.obj"),
    m_ViewController(m_GLFWHandle.window(), m_nWindowWidth, m_nWindowHeight)
{
    (void) argc;
    ImGui::GetIO().IniFilename = strdup(m_ImGuiIniFilename.c_str()); // At exit, ImGUI will store its windows positions in this file
    glEnable(GL_DEPTH_TEST);
}
