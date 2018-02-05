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


    GLDeferredShadingPassProgram::LightingUniforms lighting;
    lighting.dirLightDir = vec3(-1,-1,-1);
    lighting.dirLightIntensity = vec3(1,1,1);
    lighting.pointLightCount = 2;
    for(size_t i=0 ; i<GLForwardRenderingProgram::MAX_POINT_LIGHTS ; ++i) {
        lighting.pointLightPosition[i] = vec3(i, i*2, 1);
        lighting.pointLightIntensity[i] = vec3(1, 1, 1);
        lighting.pointLightRange[i] = 10;
        lighting.pointLightAttenuationFactor[i] = 1;
    }

    const float maxCameraSpeed = m_Scene.getDiagonalLength() * 0.5f;
    float cameraSpeed = maxCameraSpeed / 5.f;
    m_ViewController.setSpeed(cameraSpeed);

    SceneInstanceData sceneInstance;
    sceneInstance.modelMatrix = translate(mat4(1), vec3(2,0,-2));

    int currentGBufferTextureType = GNormal;

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render everything
        m_DeferredGPassProgram.use();
        m_DeferredGPassProgram.resetMaterialUniforms();
        m_Scene.render(m_DeferredGPassProgram, m_ViewController, sceneInstance);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_Fbo);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + currentGBufferTextureType);
        const GLint sx0 = 0, sy0 = 0, dx0 = 0, dy0 = 0;
        const GLint sx1 = m_nWindowWidth, sy1 = m_nWindowHeight, dx1 = sx1, dy1 = sy1;
        glBlitFramebuffer(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);


        // XXX
        // m_ForwardProgram.setLightingUniforms(lighting, m_ViewController);



        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", &clearColor[0])) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }
            ImGui::RadioButton("GPosition"       , &currentGBufferTextureType, GPosition);        ImGui::SameLine();
            ImGui::RadioButton("GNormal"         , &currentGBufferTextureType, GNormal);          ImGui::SameLine();
            ImGui::RadioButton("GAmbient"        , &currentGBufferTextureType, GAmbient);         ImGui::SameLine();
            ImGui::RadioButton("GDiffuse"        , &currentGBufferTextureType, GDiffuse);         ImGui::SameLine();
            ImGui::RadioButton("GGlossyShininess", &currentGBufferTextureType, GGlossyShininess); ImGui::SameLine();
            ImGui::RadioButton("GDepth"          , &currentGBufferTextureType, GDepth);

            if(ImGui::SliderFloat("Camera speed", &cameraSpeed, 0.001f, maxCameraSpeed)) {
                m_ViewController.setSpeed(cameraSpeed);
            }
#define EDIT_COLOR(c) ImGui::ColorEdit3(#c, &c[0])
#define EDIT_DIRECTION(c, min, max) ImGui::SliderFloat3(#c, &c[0], min, max)
            EDIT_COLOR(lighting.dirLightIntensity);
            EDIT_COLOR(lighting.pointLightIntensity[0]);
            EDIT_DIRECTION(lighting.dirLightDir, -1, 1);
            auto bound = m_Scene.getDiagonalLength() / 2.f;
            EDIT_DIRECTION(lighting.pointLightPosition[0], -bound, bound);
            ImGui::SliderFloat("near", &m_ViewController.m_Near, 0.0001f, 1.f);
            ImGui::SliderFloat("far", &m_ViewController.m_Far, 100.f, 10000.f);
            ImGui::SliderFloat("Point Light range", &lighting.pointLightRange[0], 0.01f, 1000);
            ImGui::SliderFloat("Point Light attenuation factor", &lighting.pointLightAttenuationFactor[0], 0, 100.f);
#undef EDIT_DIRECTION
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
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_DeferredGPassProgram(
        m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl",
        m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl"
    ),
    m_DeferredShadingPassProgram(
        m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl",
        m_ShadersRootPath / m_AppName / "shadingPass.fs.glsl"
    ),
    m_Scene(m_AssetsRootPath / "glmlv" / "models" / "crytek-sponza" / "sponza.obj"),
    m_ViewController(m_GLFWHandle.window(), m_nWindowWidth, m_nWindowHeight),
    m_GBufferTextures {
        { static_GBufferTextureFormat[0], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight },
        { static_GBufferTextureFormat[1], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight },
        { static_GBufferTextureFormat[2], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight },
        { static_GBufferTextureFormat[3], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight },
        { static_GBufferTextureFormat[4], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight },
        { static_GBufferTextureFormat[5], (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight }
    },
    m_Fbo(0)
{
    (void) argc;
    static_ImGuiIniFilename = m_AppName + ".imgui.ini";
    ImGui::GetIO().IniFilename = static_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

    glEnable(GL_DEPTH_TEST);

    glGenFramebuffers(1, &m_Fbo);
    assert(m_Fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBufferTextures[0].glId(), 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBufferTextures[1].glId(), 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBufferTextures[2].glId(), 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_GBufferTextures[3].glId(), 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_GBufferTextures[4].glId(), 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, m_GBufferTextures[5].glId(), 0);

    const GLenum drawBuffers[] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4
    };
    glDrawBuffers(5, drawBuffers);
    GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
#define CASE(x) case x: std::cerr << "Invalid Framebuffer: " << #x << std::endl; throw std::runtime_error("Invalid Framebuffer"); break;
    switch(status) {
    case GL_FRAMEBUFFER_COMPLETE: std::cout << "Framebuffer OK" << std::endl; break;
    case 0: std::cerr << "Framebuffer status: Unknown error" << std::endl; break;
    CASE(GL_FRAMEBUFFER_UNDEFINED);
    CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
    CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
    CASE(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
    CASE(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
    CASE(GL_FRAMEBUFFER_UNSUPPORTED);
    CASE(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
    CASE(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
    }
#undef CASE
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

std::string Application::static_ImGuiIniFilename;
const GLenum Application::static_GBufferTextureFormat[GBufferTextureCount] = {
    GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F
};

