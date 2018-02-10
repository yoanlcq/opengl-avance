#include "Demo.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>


std::string Demo::static_ImGuiIniFilename;

void handleFboStatus(GLenum status) {
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
}


using namespace glm;
using namespace glmlv;


int Demo::run()
{
    m_Deferred.m_Lighting.dirLightDir = vec3(-1,-1,-1);
    m_Deferred.m_Lighting.dirLightIntensity = vec3(1,1,1);
    m_Deferred.m_Lighting.pointLightCount = 2;
    for(size_t i=0 ; i<GLDeferredShadingPassProgram::MAX_POINT_LIGHTS ; ++i) {
        m_Deferred.m_Lighting.pointLightPosition[i] = vec3(i, i*2, 1);
        m_Deferred.m_Lighting.pointLightIntensity[i] = vec3(1, 1, 1);
        m_Deferred.m_Lighting.pointLightRange[i] = 10;
        m_Deferred.m_Lighting.pointLightAttenuationFactor[i] = 1;
    }
    m_Deferred.m_Lighting.dirLightShadowMapBias = 0.05f;

    const float maxCameraSpeed = m_Sponza.getDiagonalLength() * 0.5f;
    float cameraSpeed = maxCameraSpeed / 5.f;
    m_Camera.setSpeed(cameraSpeed);

    // Loop until the user closes the window
    for (auto frameCount = 0u; !m_GLFWHandle.shouldClose(); ++frameCount)
    {
        const auto seconds = glfwGetTime();

        m_Deferred.m_Lighting.dirLightDir = vec3(
            cos(radians(m_DirLightPhiAngleDegrees)) * sin(radians(m_DirLightThetaAngleDegrees)),
            sin(radians(m_DirLightPhiAngleDegrees)) * sin(radians(m_DirLightThetaAngleDegrees)),
            cos(radians(m_DirLightThetaAngleDegrees))
        );

        const auto computeDirectionVectorUp = [](float phiRadians, float thetaRadians) {
            const auto cosPhi = glm::cos(phiRadians);
            const auto sinPhi = glm::sin(phiRadians);
            const auto cosTheta = glm::cos(thetaRadians);
            return -glm::normalize(glm::vec3(sinPhi * cosTheta, -glm::sin(thetaRadians), cosPhi * cosTheta));
        };
        const auto sceneCenter = (m_Sponza.m_ObjData.bboxMin + m_Sponza.m_ObjData.bboxMax) / 2.f;
        const float sceneRadius = m_Sponza.getDiagonalLength() / 2.f;

        const auto dirLightUpVector = computeDirectionVectorUp(radians(m_DirLightPhiAngleDegrees), radians(m_DirLightThetaAngleDegrees));
        const auto dirLightViewMatrix = glm::lookAt(sceneCenter + m_Deferred.m_Lighting.dirLightDir * sceneRadius, sceneCenter, dirLightUpVector); // Will not work if m_Deferred.m_Lighting.dirLightDir is colinear to lightUpVector
        const auto dirLightProjMatrix = glm::ortho(-sceneRadius, sceneRadius, -sceneRadius, sceneRadius, 0.01f * sceneRadius, 2.f * sceneRadius);

        // Render shadow map
        if(m_ShadowMapping.m_IsDirty) {
            m_ShadowMapping.m_IsDirty = false;
            m_ShadowMapping.m_Program.use();
            m_ShadowMapping.m_Program.setUniformDirLightViewProjMatrix(dirLightProjMatrix * dirLightViewMatrix);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ShadowMapping.m_Fbo);
            glViewport(0, 0, m_ShadowMapping.m_Resolution, m_ShadowMapping.m_Resolution);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            m_Sponza.render();
            glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
        }

        // Geometry Pass
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Deferred.m_GBufferFbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_Deferred.m_GPassProgram.use();
        m_Deferred.m_GPassProgram.resetMaterialUniforms();
        m_Sponza.render(m_Deferred.m_GPassProgram, m_Camera, m_SponzaInstanceData);

        // Shading Pass
        if(m_PostFX.m_IsEnabled)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_PostFX.m_Input.m_Fbo);
        else
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(m_Deferred.m_GuiDisplaysGBufferTextures) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_Deferred.m_GBufferFbo);
            glReadBuffer(GL_COLOR_ATTACHMENT0 + m_Deferred.m_GuiGBufferTexIndex);
            const GLint sx0 = 0, sy0 = 0, dx0 = 0, dy0 = 0;
            const GLint sx1 = m_nWindowWidth, sy1 = m_nWindowHeight, dx1 = sx1, dy1 = sy1;
            glBlitFramebuffer(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        } else if(m_ShadowMapping.m_GuiDisplaysShadowMap) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            m_ShadowMapping.m_DisplayDepthMapProgram.use();
            m_ShadowMapping.m_DisplayDepthMapProgram.setUniformGDepth(0);
            glActiveTexture(GL_TEXTURE0);
            m_ShadowMapping.m_Texture.bind();
            m_ShadowMapping.m_Sampler.bindToTextureUnit(0);
            m_ScreenCoverQuad.render();
        } else {
            for(GLuint i=0 ; i<GBufferTextureCount ; ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                m_Deferred.m_GBufferTextures[i].bind();
            }
            m_Deferred.m_ShadingPassProgram.use();
            m_Deferred.m_Lighting.dirLightShadowMap = GBufferTextureCount;
            glActiveTexture(GL_TEXTURE0 + m_Deferred.m_Lighting.dirLightShadowMap);
            m_ShadowMapping.m_Texture.bind();
            m_ShadowMapping.m_Sampler.bindToTextureUnit(m_Deferred.m_Lighting.dirLightShadowMap);
            m_Deferred.m_Lighting.dirLightViewProjMatrix = dirLightProjMatrix * dirLightViewMatrix * m_Camera.getRcpViewMatrix();
            m_Deferred.m_Lighting.dirLightDir = -m_Deferred.m_Lighting.dirLightDir;
            m_Deferred.m_ShadingPassProgram.setLightingUniforms(m_Deferred.m_Lighting, m_Camera);
            m_Deferred.m_Lighting.dirLightDir = -m_Deferred.m_Lighting.dirLightDir;
            m_Deferred.m_ShadingPassProgram.setUniformGPosition(0);
            m_Deferred.m_ShadingPassProgram.setUniformGNormal(1);
            m_Deferred.m_ShadingPassProgram.setUniformGAmbient(2);
            m_Deferred.m_ShadingPassProgram.setUniformGDiffuse(3);
            m_Deferred.m_ShadingPassProgram.setUniformGGlossyShininess(4);
            m_ScreenCoverQuad.render();
        }

        if(m_PostFX.m_IsEnabled) {
            m_PostFX.m_Program.use();
            m_PostFX.m_Program.setUniformGammaExponent(1.f / m_PostFX.m_Gamma);
            m_PostFX.m_Program.setUniformInputImage(0);
            m_PostFX.m_Program.setUniformOutputImage(1);
            glBindImageTexture(0, m_PostFX.m_Input.m_Texture.glId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, m_PostFX.m_Output.m_Texture.glId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            // NOTE!!!! 32 = local_size dans le compute shader. 
            glDispatchCompute(1 + m_nWindowWidth / 32, 1 + m_nWindowHeight / 32, 1);
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_PostFX.m_Output.m_Fbo);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            const GLint sx0 = 0, sy0 = 0, dx0 = 0, dy0 = 0;
            const GLint sx1 = m_nWindowWidth, sy1 = m_nWindowHeight, dx1 = sx1, dy1 = sy1;
            glBlitFramebuffer(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        }

        // GUI code:
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        ImGui_ImplGlfwGL3_NewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", &m_ClearColor[0])) {
                glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.f);
            }
            ImGui::SliderFloat("Gamma", &m_PostFX.m_Gamma, 0, 16);
            if(ImGui::Button(m_Deferred.m_GuiDisplaysGBufferTextures ? "Hide G-Buffers" : "Show G-Buffers")) {
                m_Deferred.m_GuiDisplaysGBufferTextures = !m_Deferred.m_GuiDisplaysGBufferTextures;
            }
            ImGui::SameLine();
            if(ImGui::Button(m_ShadowMapping.m_GuiDisplaysShadowMap ? "Hide Shadow Map" : "Show Shadow Map")) {
                m_ShadowMapping.m_GuiDisplaysShadowMap = !m_ShadowMapping.m_GuiDisplaysShadowMap;
            }
            if(ImGui::Button(m_PostFX.m_IsEnabled ? "Disable post-processing" : "Enable post-processing")) {
                m_PostFX.m_IsEnabled = !m_PostFX.m_IsEnabled;
            }
            if(m_Deferred.m_GuiDisplaysGBufferTextures) {
                ImGui::RadioButton("GPosition"       , &m_Deferred.m_GuiGBufferTexIndex, GPosition);        ImGui::SameLine();
                ImGui::RadioButton("GNormal"         , &m_Deferred.m_GuiGBufferTexIndex, GNormal);          ImGui::SameLine();
                ImGui::RadioButton("GAmbient"        , &m_Deferred.m_GuiGBufferTexIndex, GAmbient);         ImGui::SameLine();
                ImGui::RadioButton("GDiffuse"        , &m_Deferred.m_GuiGBufferTexIndex, GDiffuse);         ImGui::SameLine();
                ImGui::RadioButton("GGlossyShininess", &m_Deferred.m_GuiGBufferTexIndex, GGlossyShininess); ImGui::SameLine();
                ImGui::RadioButton("GDepth"          , &m_Deferred.m_GuiGBufferTexIndex, GDepth);
            }
            ImGui::Text("Shadow Map Resolution:");
            static const GLuint resolutions[8] = { 16, 32, 64, 128, 256, 512, 1024, 2048 };
            for(int i=0 ; i<8 ; ++i) {
                char txt[8];
                sprintf(txt, "%u", resolutions[i]);
                if(ImGui::RadioButton(txt, &m_ShadowMapping.m_Resolution, resolutions[i])) {
                    m_ShadowMapping.m_IsDirty = true;
                    m_ShadowMapping.m_Texture = GLTexture2D(GL_DEPTH_COMPONENT32F, m_ShadowMapping.m_Resolution, m_ShadowMapping.m_Resolution);
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ShadowMapping.m_Fbo);
                    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowMapping.m_Texture.glId(), 0);
                    handleFboStatus(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                }
                if(i < 8-1) {
                    ImGui::SameLine();
                }
            }

            if(ImGui::SliderFloat("Camera speed", &cameraSpeed, 0.001f, maxCameraSpeed)) {
                m_Camera.setSpeed(cameraSpeed);
            }
#define EDIT_COLOR(c) ImGui::ColorEdit3(#c, &c[0])
#define EDIT_DIRECTION(c, min, max) ImGui::SliderFloat3(#c, &c[0], min, max)
            EDIT_COLOR(m_Deferred.m_Lighting.dirLightIntensity);
            EDIT_COLOR(m_Deferred.m_Lighting.pointLightIntensity[0]);
            auto bound = m_Sponza.getDiagonalLength() / 2.f;
            EDIT_DIRECTION(m_Deferred.m_Lighting.pointLightPosition[0], -bound, bound);
            if(ImGui::SliderFloat("dirLight Phi", &m_DirLightPhiAngleDegrees, 0, 360))
                m_ShadowMapping.m_IsDirty = true;
            if(ImGui::SliderFloat("dirLight Theta", &m_DirLightThetaAngleDegrees, 0, 360))
                m_ShadowMapping.m_IsDirty = true;
            ImGui::Text(m_ShadowMapping.m_IsDirty ? "Shadow Map is dirty" : "Shadow Map is not dirty");
            ImGui::SliderFloat("SM Bias", &m_Deferred.m_Lighting.dirLightShadowMapBias, 0, 10.f);
            ImGui::SliderInt("SM Sample Count", &m_Deferred.m_Lighting.dirLightShadowMapSampleCount, 1, 128);
            ImGui::SliderFloat("SM Spread", &m_Deferred.m_Lighting.dirLightShadowMapSpread, 0, 0.01f);
            ImGui::SliderFloat("near", &m_Camera.m_Near, 0.0001f, 1.f);
            ImGui::SliderFloat("far", &m_Camera.m_Far, 100.f, 10000.f);
            ImGui::SliderFloat("Point Light range", &m_Deferred.m_Lighting.pointLightRange[0], 0.01f, 1000);
            ImGui::SliderFloat("Point Light attenuation factor", &m_Deferred.m_Lighting.pointLightAttenuationFactor[0], 0, 100.f);
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
            m_Camera.update(float(elapsedTime));
        }
    }

    return 0;
}

Demo::Demo(int argc, char** argv):
    m_Paths(glmlv::fs::path{ argv[0] }),
    m_Deferred(m_Paths, (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight),
    m_ShadowMapping(m_Paths),
    m_PostFX(m_Paths, (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight),
    m_ClearColor(0, 186/255.f, 1.f),
    m_DirLightPhiAngleDegrees(260),
    m_DirLightThetaAngleDegrees(260),
    m_Camera(m_GLFWHandle.window(), m_nWindowWidth, m_nWindowHeight),
    m_Sponza(m_Paths.m_AssetsRoot / "glmlv" / "models" / "crytek-sponza" / "sponza.obj"),
    m_ScreenCoverQuad(glmlv::makeScreenCoverQuad())
{
    (void) argc;
    static_ImGuiIniFilename = m_Paths.m_AppName + ".imgui.ini";
    ImGui::GetIO().IniFilename = static_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

    glEnable(GL_DEPTH_TEST);
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.f);
}
