#include "Demo.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>

#include <glmlv/OpenSimplexNoise.hpp>


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

void Demo::renderGUI() {
    ImGui_ImplGlfwGL3_NewFrame();


    ImGui::Begin("GUI");
    ImGui::ColorEditMode(ImGuiColorEditMode_RGB);

    ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    if(ImGui::CollapsingHeader("ImGUI Style Editor")) {
        ImGui::ShowStyleEditor();
    }
    if(ImGui::CollapsingHeader("Clear Color")) {
        if (ImGui::ColorEdit3("clearColor", &m_ClearColor[0])) {
            glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.f);
        }
    }
    if(ImGui::CollapsingHeader("Camera")) {
        if(ImGui::SliderFloat("Speed", &m_CameraSpeed, 0.001f, m_CameraMaxSpeed)) {
            m_Camera.setSpeed(m_CameraSpeed);
        }
        ImGui::SliderFloat("Vertical FOV", &m_Camera.m_FovY, 0.01f, radians(179.f));
        ImGui::SliderFloat("Near", &m_Camera.m_Near, 0.0001f, 1.f);
        ImGui::SliderFloat("Far", &m_Camera.m_Far, 100.f, 10000.f);
        ImGui::SliderFloat("Skybox scale", &m_Skybox.m_Scale, 1.f, m_Sponza.getDiagonalLength() * 2.f);
    }
    ImGui::Text("Pipeline: ");
    ImGui::RadioButton("Forward", &m_PipelineKind, PIPELINE_FORWARD);
    ImGui::RadioButton("Deferred", &m_PipelineKind, PIPELINE_DEFERRED);

    switch(m_PipelineKind) {
    case PIPELINE_FORWARD:
        break;
    case PIPELINE_DEFERRED:
        if(ImGui::CollapsingHeader("Deferred Rendering")) {
            ImGui::Checkbox("Debug G-Buffers", &m_DeferredRendering.m_GuiDisplaysGBufferTextures);
            ImGui::Text("Which one:");
            ImGui::RadioButton("GPosition"       , &m_DeferredRendering.m_GuiGBufferTexIndex, GPosition);       
            ImGui::RadioButton("GNormal"         , &m_DeferredRendering.m_GuiGBufferTexIndex, GNormal);         
            ImGui::RadioButton("GAmbient"        , &m_DeferredRendering.m_GuiGBufferTexIndex, GAmbient);        
            ImGui::RadioButton("GDiffuse"        , &m_DeferredRendering.m_GuiGBufferTexIndex, GDiffuse);        
            ImGui::RadioButton("GGlossyShininess", &m_DeferredRendering.m_GuiGBufferTexIndex, GGlossyShininess);
            ImGui::RadioButton("GDepth"          , &m_DeferredRendering.m_GuiGBufferTexIndex, GDepth);
        }
        break;
    }
    if(ImGui::CollapsingHeader("Point Lights")) {
        const float sceneBoundary = m_Sponza.getDiagonalLength() / 2.f;
        ImGui::SliderInt("Count", &m_Lighting.pointLightCount, 0, GLDeferredShadingPassProgram::MAX_POINT_LIGHTS);
        ImGui::Indent();
        for(int i=0 ; i<m_Lighting.pointLightCount ; ++i) {
            char txt[32];
            snprintf(txt, sizeof txt, "Point Light n°%i", i);
            ImGui::PushID(i);
            if(ImGui::CollapsingHeader(txt)) {
                ImGui::ColorEdit3("Intensity", &m_Lighting.pointLightIntensity[i][0]);
                ImGui::SliderFloat3("Position", &m_Lighting.pointLightPosition[i][0], -sceneBoundary, sceneBoundary);
                ImGui::SliderFloat("Range", &m_Lighting.pointLightRange[i], 0.01f, 1000);
                ImGui::SliderFloat("Attenuation Factor", &m_Lighting.pointLightAttenuationFactor[i], 0, 100.f);
            }
            ImGui::PopID();
        }
        ImGui::Unindent();
    }
    if(ImGui::CollapsingHeader("Directional Light")) {
        ImGui::ColorEdit3("Intensity", &m_Lighting.dirLightIntensity[0]);
        if(ImGui::SliderFloat("Phi Angle", &m_DirLightPhiAngleDegrees, 0, 360))
            m_DirectionalShadowMapping.m_IsDirty = true;
        if(ImGui::SliderFloat("Theta Angle", &m_DirLightThetaAngleDegrees, 0, 360))
            m_DirectionalShadowMapping.m_IsDirty = true;

        ImGui::Indent();
        if(ImGui::CollapsingHeader("Directional Shadow Mapping")) {
            ImGui::Checkbox("Show Shadow Map", &m_DirectionalShadowMapping.m_GuiDisplaysShadowMap);
            ImGui::Text(m_DirectionalShadowMapping.m_IsDirty ? "Shadow Map is dirty" : "Shadow Map is not dirty");
            ImGui::SliderFloat("Bias", &m_Lighting.dirLightShadowMapBias, 0, 10.f);
            ImGui::SliderInt("Sample Count", &m_Lighting.dirLightShadowMapSampleCount, 1, 128);
            ImGui::SliderFloat("Spread", &m_Lighting.dirLightShadowMapSpread, 0, 0.01f);
            ImGui::Text("Resolution:");
            static const GLuint resolutions[8] = { 16, 32, 64, 128, 256, 512, 1024, 2048 };
            for(int i=0 ; i<8 ; ++i) {
                char txt[8];
                sprintf(txt, "%u", resolutions[i]);
                if(ImGui::RadioButton(txt, &m_DirectionalShadowMapping.m_Resolution, resolutions[i])) {
                    m_DirectionalShadowMapping.m_IsDirty = true;
                    m_DirectionalShadowMapping.m_Texture = GLTexture2D(GL_DEPTH_COMPONENT32F, m_DirectionalShadowMapping.m_Resolution, m_DirectionalShadowMapping.m_Resolution);
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_DirectionalShadowMapping.m_Fbo);
                    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DirectionalShadowMapping.m_Texture.glId(), 0);
                    handleFboStatus(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                }
                if(i < 8-1) {
                    ImGui::SameLine();
                }
            }
        }
        ImGui::Unindent();
    }

    auto& fpass = m_PostFX.m_FragmentPass;

    if(ImGui::CollapsingHeader("PostFX: Fragment Pass")) {
        ImGui::Checkbox("Enable Fragment Pass", &fpass.m_IsEnabled);
        if(fpass.m_IsEnabled) {
            ImGui::Text("Blur: "); ImGui::SameLine();
            ImGui::RadioButton("None"  , &fpass.m_BlurTechnique, PostFX_FragmentPassProgram::BLUR_NONE); ImGui::SameLine();
            ImGui::RadioButton("Box"   , &fpass.m_BlurTechnique, PostFX_FragmentPassProgram::BLUR_BOX);  ImGui::SameLine();
            ImGui::RadioButton("Radial", &fpass.m_BlurTechnique, PostFX_FragmentPassProgram::BLUR_RADIAL);
            switch(fpass.m_BlurTechnique) {
            case PostFX_FragmentPassProgram::BLUR_NONE:
                break;
            case PostFX_FragmentPassProgram::BLUR_BOX: 
                ImGui::SliderInt("Matrix Half Side", &fpass.m_BoxBlurMatrixHalfSide, 0, 8);
                {
                    int s = fpass.m_BoxBlurMatrixHalfSide;
                    ImGui::Text("%ix%i matrix", s+1+s, s+1+s);
                }
                break;
            case PostFX_FragmentPassProgram::BLUR_RADIAL:
                ImGui::SliderInt("Num Samples",  &fpass.m_RadialBlurNumSamples, 1, 64);
                ImGui::SliderFloat("Max Length", &fpass.m_RadialBlurMaxLength, 0, 1.0f);
                break;
            }
            ImGui::Checkbox("Enable Bloom", &fpass.m_BloomEnabled);
            ImGui::SliderInt("Bloom Matrix Half Side", &fpass.m_BloomMatrixHalfSide, 0, 8);
            {
                int s = fpass.m_BoxBlurMatrixHalfSide;
                ImGui::Text("%ix%i matrix", s+1+s, s+1+s);
            }
            ImGui::SliderFloat("Bloom Texel Skip", &fpass.m_BloomTexelSkip, 0, 20.0f);
            ImGui::SliderFloat("Bloom Threshold", &fpass.m_BloomThreshold, 0, 1.0f);
        }
    }

    auto& cpass = m_PostFX.m_ComputePass;

    if(ImGui::CollapsingHeader("PostFX: Compute Pass")) {
        ImGui::Checkbox("Enable Compute Pass", &cpass.m_IsEnabled);
        if(cpass.m_IsEnabled) {
            ImGui::SliderFloat("Gamma", &cpass.m_Gamma, 0, 16);
            ImGui::SliderFloat3("Final Touch Mul", &cpass.m_FinalTouchMul[0], -2, 2);
            ImGui::SliderFloat3("Final Touch Add", &cpass.m_FinalTouchAdd[0], -2, 2);
        }
    }

    ImGui::End();

    const auto viewportSize = m_GLFWHandle.framebufferSize();
    glViewport(0, 0, viewportSize.x, viewportSize.y);
    ImGui::Render();
}


void Demo::renderGeometry() {
    m_Sponza.render();
    m_Skybox.render(m_Camera);
}
void Demo::renderGeometry(const GLMaterialProgram& prog) {
    m_Sponza.render(prog, m_Camera, m_SponzaInstanceData);
    m_Skybox.render(m_Camera);
}

void Demo::renderFrame() {
    m_Lighting.dirLightDir = vec3(
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
    const auto dirLightViewMatrix = glm::lookAt(sceneCenter + m_Lighting.dirLightDir * sceneRadius, sceneCenter, dirLightUpVector); // Will not work if m_Lighting.dirLightDir is colinear to lightUpVector
    const auto dirLightProjMatrix = glm::ortho(-sceneRadius, sceneRadius, -sceneRadius, sceneRadius, 0.01f * sceneRadius, 2.f * sceneRadius);

    // Render shadow map
    if(m_DirectionalShadowMapping.m_IsDirty) {
        m_DirectionalShadowMapping.m_IsDirty = false;
        m_DirectionalShadowMapping.m_Program.use();
        m_DirectionalShadowMapping.m_Program.setUniformDirLightViewProjMatrix(dirLightProjMatrix * dirLightViewMatrix);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_DirectionalShadowMapping.m_Fbo);
        glViewport(0, 0, m_DirectionalShadowMapping.m_Resolution, m_DirectionalShadowMapping.m_Resolution);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderGeometry();
        glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
    }

    switch(m_PipelineKind) {
    case PIPELINE_FORWARD:
        break;
    case PIPELINE_DEFERRED:
        // Geometry Pass
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_DeferredRendering.m_GBufferFbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_DeferredRendering.m_GPassProgram.use();
        m_DeferredRendering.m_GPassProgram.resetMaterialUniforms();
        renderGeometry(m_DeferredRendering.m_GPassProgram);
        break;
    }

    auto& fpass = m_PostFX.m_FragmentPass;
    auto& cpass = m_PostFX.m_ComputePass;

    GLuint nextRenderTarget = 0;
    /**/ if(fpass.m_IsEnabled) nextRenderTarget = fpass.m_HiRes.m_Fbo;
    else if(cpass.m_IsEnabled) nextRenderTarget = cpass.m_Input.m_Fbo;

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, nextRenderTarget);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch(m_PipelineKind) {
    case PIPELINE_FORWARD:
        m_ForwardRendering.m_Program.use();
        m_Lighting.dirLightShadowMap = m_Sponza.m_GLTextures2D.size()+2; // XXX HACK
        glActiveTexture(GL_TEXTURE0 + m_Lighting.dirLightShadowMap);
        m_DirectionalShadowMapping.m_Texture.bind();
        m_DirectionalShadowMapping.m_Sampler.bindToTextureUnit(m_Lighting.dirLightShadowMap);
        m_Lighting.dirLightViewProjMatrix = dirLightProjMatrix * dirLightViewMatrix * m_Camera.getRcpViewMatrix();
        m_Lighting.dirLightDir = -m_Lighting.dirLightDir; // FIXME Hack!!!!!!
        m_ForwardRendering.m_Program.setLightingUniforms(m_Lighting, m_Camera);
        m_Lighting.dirLightDir = -m_Lighting.dirLightDir; // FIXME Hack!!!!!!
        m_ForwardRendering.m_Program.resetMaterialUniforms();
        renderGeometry(m_ForwardRendering.m_Program);
        break;
    case PIPELINE_DEFERRED:
        // Shading Pass
        if(m_DeferredRendering.m_GuiDisplaysGBufferTextures) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_DeferredRendering.m_GBufferFbo);
            glReadBuffer(GL_COLOR_ATTACHMENT0 + m_DeferredRendering.m_GuiGBufferTexIndex);
            const GLint w = m_nWindowWidth, h = m_nWindowHeight;
            glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        } else if(m_DirectionalShadowMapping.m_GuiDisplaysShadowMap) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            m_DirectionalShadowMapping.m_DisplayDepthMapProgram.use();
            m_DirectionalShadowMapping.m_DisplayDepthMapProgram.setUniformGDepth(0);
            glActiveTexture(GL_TEXTURE0);
            m_DirectionalShadowMapping.m_Texture.bind();
            m_DirectionalShadowMapping.m_Sampler.bindToTextureUnit(0);
            m_ScreenCoverQuad.render();
        } else {
            for(GLuint i=0 ; i<GBufferTextureCount ; ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                m_DeferredRendering.m_GBufferTextures[i].bind();
            }
            m_DeferredRendering.m_ShadingPassProgram.use();
            m_Lighting.dirLightShadowMap = GBufferTextureCount;
            glActiveTexture(GL_TEXTURE0 + m_Lighting.dirLightShadowMap);
            m_DirectionalShadowMapping.m_Texture.bind();
            m_DirectionalShadowMapping.m_Sampler.bindToTextureUnit(m_Lighting.dirLightShadowMap);
            m_Lighting.dirLightViewProjMatrix = dirLightProjMatrix * dirLightViewMatrix * m_Camera.getRcpViewMatrix();
            m_Lighting.dirLightDir = -m_Lighting.dirLightDir; // FIXME Hack!!!!!!
            m_DeferredRendering.m_ShadingPassProgram.setLightingUniforms(m_Lighting, m_Camera);
            m_Lighting.dirLightDir = -m_Lighting.dirLightDir;
            m_DeferredRendering.m_ShadingPassProgram.setUniformGPosition(0);
            m_DeferredRendering.m_ShadingPassProgram.setUniformGNormal(1);
            m_DeferredRendering.m_ShadingPassProgram.setUniformGAmbient(2);
            m_DeferredRendering.m_ShadingPassProgram.setUniformGDiffuse(3);
            m_DeferredRendering.m_ShadingPassProgram.setUniformGGlossyShininess(4);
            m_ScreenCoverQuad.render();
        }
        break;
    }

    if(fpass.m_IsEnabled) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fpass.m_LoRes.m_Fbo);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fpass.m_HiRes.m_Fbo);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        const GLint w = m_nWindowWidth, h = m_nWindowHeight;
        const GLint dw = fpass.m_LoResWidth, dh = fpass.m_LoResHeight;
        glBlitFramebuffer(0, 0, w, h, 0, 0, dw, dh, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cpass.m_IsEnabled ? cpass.m_Input.m_Fbo : 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        fpass.m_HiRes.m_Texture.bind();
        glActiveTexture(GL_TEXTURE1);
        fpass.m_LoRes.m_Texture.bind();
        fpass.m_NearestSampler.bindToTextureUnit(0);
        fpass.m_LinearSampler.bindToTextureUnit(1);
        fpass.m_Program.use();
        fpass.m_Program.setUniformHiResTexture(0);
        fpass.m_Program.setUniformLoResTexture(1);
        fpass.m_Program.setUniformWindowSize((GLuint) m_nWindowWidth, (GLuint) m_nWindowHeight);
        fpass.m_Program.setUniformBlurTechnique(fpass.m_BlurTechnique);
        fpass.m_Program.setUniformBoxBlurMatrixHalfSide(fpass.m_BoxBlurMatrixHalfSide);
        fpass.m_Program.setUniformRadialBlurNumSamples(fpass.m_RadialBlurNumSamples);
        fpass.m_Program.setUniformRadialBlurMaxLength(fpass.m_RadialBlurMaxLength);
        fpass.m_Program.setUniformBloomEnabled(fpass.m_BloomEnabled);
        fpass.m_Program.setUniformBloomMatrixHalfSide(fpass.m_BloomMatrixHalfSide);
        fpass.m_Program.setUniformBloomTexelSkip(fpass.m_BloomTexelSkip);
        fpass.m_Program.setUniformBloomThreshold(fpass.m_BloomThreshold);
        m_ScreenCoverQuad.render();
    }

    if(cpass.m_IsEnabled) {
		glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindImageTexture(0, cpass.m_Input.m_Texture.glId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, cpass.m_Output.m_Texture.glId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        cpass.m_Program.use();
        cpass.m_Program.setUniformInputImage(0);
        cpass.m_Program.setUniformOutputImage(1);
        cpass.m_Program.setUniformGammaExponent(1.f / cpass.m_Gamma);
        cpass.m_Program.setUniformFinalTouchMul(cpass.m_FinalTouchMul);
        cpass.m_Program.setUniformFinalTouchAdd(cpass.m_FinalTouchAdd);
        // NOTE!!!! 32 = local_size dans le compute shader. 
        glDispatchCompute(1 + m_nWindowWidth / 32, 1 + m_nWindowHeight / 32, 1);
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, cpass.m_Output.m_Fbo);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        const GLint w = m_nWindowWidth, h = m_nWindowHeight;
        glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }
}

int Demo::run()
{
    // Loop until the user closes the window
    for (auto frameCount = 0u; !m_GLFWHandle.shouldClose(); ++frameCount)
    {
        const auto seconds = glfwGetTime();

        renderFrame();

        // GUI code:
        renderGUI();

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
    m_PipelineKind(PIPELINE_FORWARD),
    m_ForwardRendering(m_Paths),
    m_DeferredRendering(m_Paths, (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight),
    m_DirectionalShadowMapping(m_Paths),
    m_PostFX(m_Paths, (GLsizei) m_nWindowWidth, (GLsizei) m_nWindowHeight),
    m_ClearColor(0, 186/255.f, 1.f),
    m_Lighting(),
    m_DirLightPhiAngleDegrees(260),
    m_DirLightThetaAngleDegrees(260),
    m_ScreenCoverQuad(glmlv::makeScreenCoverQuad()),
    m_Sponza(m_Paths.m_AssetsRoot / "glmlv" / "models" / "crytek-sponza" / "sponza.obj"),
    m_SponzaInstanceData(),
    m_Camera(m_GLFWHandle.window(), m_nWindowWidth, m_nWindowHeight),
    m_CameraMaxSpeed(m_Sponza.getDiagonalLength() / 2.f),
    m_CameraSpeed(m_CameraMaxSpeed / 5.f),
    m_Skybox(m_Paths, m_Sponza.getDiagonalLength() / 2.f)
{
    (void) argc;
    static_ImGuiIniFilename = m_Paths.m_AppName + ".imgui.ini";
    ImGui::GetIO().IniFilename = static_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.f);
    m_Camera.setSpeed(m_CameraSpeed);

    m_Lighting.dirLightDir = vec3(-1,-1,-1);
    m_Lighting.dirLightIntensity = vec3(1,1,1);
    m_Lighting.pointLightCount = 2;
    for(size_t i=0 ; i<GLDeferredShadingPassProgram::MAX_POINT_LIGHTS ; ++i) {
        m_Lighting.pointLightPosition[i] = vec3(i, i*2, 1);
        m_Lighting.pointLightIntensity[i] = vec3(1, 1, 1);
        m_Lighting.pointLightRange[i] = 10;
        m_Lighting.pointLightAttenuationFactor[i] = 1;
    }
    m_Lighting.dirLightShadowMapBias = 0.05f;
}
