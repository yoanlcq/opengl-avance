#include "Demo.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glm/gtc/matrix_transform.hpp>


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


// Picked using GIMP
const std::array<Rect<uint32_t>, Sprites::AtlasCount> Sprites::ATLAS_SIZE = {
    Rect<uint32_t> { 0, 0, 512, 512 },
    Rect<uint32_t> { 0, 0, 512, 512 },
};
const std::array<Sprites::AtlasIndex, Sprites::SprCount> Sprites::SPR_ATLAS_INDEX = {
    AtlasImacLogo,
    AtlasTexts,
    AtlasTexts,
    AtlasTexts,
    AtlasTexts,
    AtlasTexts,
};
const std::array<Rect<uint32_t>, Sprites::SprCount> Sprites::SPR_TEXCOORDS_UINT = {
    Rect<uint32_t> { 0, 0, 512, 512 },
    Rect<uint32_t> { 0, 48, 328, 110 - 48 },
    Rect<uint32_t> { 0, 126, 512, 177 - 126 },
    Rect<uint32_t> { 0, 203, 158, 257 - 203 },
    Rect<uint32_t> { 0, 268, 492, 328 - 268 },
    Rect<uint32_t> { 0, 347, 512, 436 - 347 },
};
const std::array<Rect<float>, Sprites::SprCount> Sprites::SPR_TEXCOORDS = {
    SPR_TEXCOORDS_UINT[0].dividedBySize(512, 512),
    SPR_TEXCOORDS_UINT[1].dividedBySize(512, 512),
    SPR_TEXCOORDS_UINT[2].dividedBySize(512, 512),
    SPR_TEXCOORDS_UINT[3].dividedBySize(512, 512),
    SPR_TEXCOORDS_UINT[4].dividedBySize(512, 512),
    SPR_TEXCOORDS_UINT[5].dividedBySize(512, 512),
};

using namespace glm;
using namespace glmlv;

Image2DRGBA readImageNoException(const fs::path& path) noexcept {
    try {
        return readImage(path);
    } catch(...) {
        std::clog << "Warning: Could not load " << path << ". Returning a 1x1 magenta image instead." << std::endl;
        return Image2DRGBA(1, 1, 255, 000, 255, 255); // Magenta
    }
}

Scene& Demo::getCurrentScene() {
    return const_cast<Scene&>(static_cast<const Demo&>(*this).getCurrentScene());
}
const Scene& Demo::getCurrentScene() const {
    switch(m_SceneID) {
    case SCENEID_END_OF_THE_WORLD: return m_Sun;
    case SCENEID_CITY: return m_City;
    }
    return m_Sun;
}

float Demo::getCurrentSceneDiagonalLength() const {
    return getCurrentScene().getDiagonalLength();
}

glm::vec3 Demo::getCurrentSceneCenter() const {
    const auto& o = getCurrentScene().m_ObjData;
    return (o.bboxMax + o.bboxMin) / 2.f;
}

vec3 Demo::getShipLeftReactorPosition() const {
    return vec3(m_ShipInstanceData.getModelMatrix() * vec4(-0.275, 0.165, 0.739, 1));
}
vec3 Demo::getShipRightReactorPosition() const {
    return vec3(m_ShipInstanceData.getModelMatrix() * vec4( 0.275, 0.165, 0.739, 1));
}


void Demo::changeSceneIDAndConfigure(int sceneID) {
    if(sceneID == m_SceneID) {
        return;
    }
    std::cout << "Changing SceneID: " << sceneID << std::endl;

    m_SceneID = sceneID;

    m_CameraMaxSpeed = getCurrentSceneDiagonalLength() / 2.f;
    m_CameraSpeed = m_CameraMaxSpeed / 5.f;
    m_DirectionalShadowMapping.m_IsDirty = true;

    switch(sceneID) {
    case SCENEID_END_OF_THE_WORLD:
        m_Skybox.m_CurrentSky = Skybox::SkySpace;
        m_Camera.m_Near = 0.2f;
        m_Camera.m_Far = 10000.f;
        m_Camera.m_FovY = radians(60.f);
        m_SceneCenterForShadowMap = vec3(0/*, -20, 0*/);
        m_SceneRadiusForShadowMap = 32.f;
        m_Lighting.dirLightIntensity = vec3(0);
        m_Lighting.pointLightCount = 4;

        // Main sun
        m_Lighting.pointLightIntensity[0] = vec3(1);
        m_Lighting.pointLightPosition[0] = vec3(0, 14, -56);
        m_Lighting.pointLightRange[0] = 75;
        m_Lighting.pointLightAttenuationFactor[0] = 1;

        // Base top
        m_Lighting.pointLightIntensity[1] = vec3(198, 80, 11) / 255.f;
        m_Lighting.pointLightPosition[1] = vec3(0, 4, -6);
        m_Lighting.pointLightRange[1] = 10;
        m_Lighting.pointLightAttenuationFactor[1] = 0.094;

        // Left reactor (see also update())
        m_Lighting.pointLightIntensity[2] = vec3(255, 134, 56) / 255.f;
        m_Lighting.pointLightPosition[2] = getShipLeftReactorPosition();
        m_Lighting.pointLightRange[2] = 0.25;
        m_Lighting.pointLightAttenuationFactor[2] = 0.039;

        // Right reactor (see also update())
        m_Lighting.pointLightIntensity[3] = vec3(255, 134, 56) / 255.f;
        m_Lighting.pointLightPosition[3] = getShipRightReactorPosition();
        m_Lighting.pointLightRange[3] = 0.25;
        m_Lighting.pointLightAttenuationFactor[3] = 0.039;

        m_ParticlesManager.m_LeftReactorParticles.m_SpawnRadius = 0.226f;
        m_ParticlesManager.m_RightReactorParticles.m_SpawnRadius = 0.226f;

        m_ParticlesManager.m_LeftReactorParticlesInstanceData.origin = getShipLeftReactorPosition();
        m_ParticlesManager.m_LeftReactorParticlesInstanceData.velMultiplier = 0.f;
        m_ParticlesManager.m_LeftReactorParticlesInstanceData.pointSize = 8;
        m_ParticlesManager.m_LeftReactorParticlesInstanceData.zScale = 1.613f;
        m_ParticlesManager.m_LeftReactorParticlesInstanceData.zInfluence = 1;

        m_ParticlesManager.m_RightReactorParticlesInstanceData.origin = getShipRightReactorPosition();
        m_ParticlesManager.m_RightReactorParticlesInstanceData.velMultiplier = 0.f;
        m_ParticlesManager.m_RightReactorParticlesInstanceData.pointSize = 8;
        m_ParticlesManager.m_RightReactorParticlesInstanceData.zScale = 1.613f;
        m_ParticlesManager.m_RightReactorParticlesInstanceData.zInfluence = 1;

        break;
    case SCENEID_CITY:
        m_Skybox.m_CurrentSky = Skybox::SkyPlanetFlashBack;
        m_Camera.m_Near = 0.001f;
        m_Camera.m_Far = 1000.f;
        m_CameraSpeed = 0.1f;
        m_Camera.m_FovY = 0.8f;
        m_SceneCenterForShadowMap = vec3(0);
        m_SceneRadiusForShadowMap = 4.2f;
        m_DirLightPhiAngleDegrees = 340.f;
        m_DirLightThetaAngleDegrees = 248.f;
        m_Lighting.dirLightShadowMapBias = 0.005f;
        m_Lighting.dirLightShadowMapSampleCount = 1;
        m_Lighting.dirLightShadowMapSpread = 0;
        m_Lighting.dirLightIntensity = vec3(1);

        m_Lighting.pointLightCount = 0; // <===== En fait ignorer la light ci-dessous

        // Une point light pour faire joli. N'est qu'un example pour en mettre
        // plusieurs.
        m_Lighting.pointLightIntensity[0] = vec3(1);
        m_Lighting.pointLightPosition[0] = vec3(0, 0, 1);
        m_Lighting.pointLightRange[0] = 0.017f;
        m_Lighting.pointLightAttenuationFactor[0] = 0.06f;
        // Other notes:
        // Cam pos: 0.0145 0.0856 -1
        // Cam forward: 0 0.05 1
        break;
    }

    m_Camera.setSpeed(m_CameraSpeed);
}


void Demo::renderGUI() {

    if(!m_Story.shouldGuiBeVisible())
        return;

    ImGui_ImplGlfwGL3_NewFrame();


    ImGui::Begin("GUI");
    ImGui::ColorEditMode(ImGuiColorEditMode_RGB);

    ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    if(ImGui::CollapsingHeader("ImGUI Style Editor")) {
        ImGui::ShowStyleEditor();
    }

    if(m_Story.isPlaying()) {
        const auto t = m_Story.getPlayheadTime();
        ImGui::Text("Playhead time : %.2f (%.2f%%)", t, 100.f * t / Story::DURATION);
        ImGui::Text("Press G to toggle GUI, SPACE to stop.");
    } else {
        ImGui::Text("NOTE: Press SPACE to toggle Demo Mode.");
        ImGui::Text("While in Demo Mode, press G to toggle GUI.");
    }
    ImGui::Text("Pipeline: ");
    ImGui::RadioButton("Forward", &m_PipelineKind, PIPELINE_FORWARD);
    ImGui::RadioButton("Deferred", &m_PipelineKind, PIPELINE_DEFERRED);

    ImGui::Text("Scene: ");
    int tmpSceneID = m_SceneID;
    ImGui::RadioButton("End Of The World", &tmpSceneID, SCENEID_END_OF_THE_WORLD);
    ImGui::RadioButton("City", &tmpSceneID, SCENEID_CITY);
    changeSceneIDAndConfigure(tmpSceneID);

    if(ImGui::CollapsingHeader("End Of The World - Data")) {

        const float mmin = -10.f, mmax = 10.f;

        ImGui::SliderFloat("Ship Reactor Strength", &m_ShipReactorStrength, 0, 1);
        ImGui::InputFloat3("Ship Position", &m_ShipInstanceData.m_Position[0]);
        ImGui::InputFloat3("Ship Forward", &m_ShipInstanceData.m_Forward[0]);
        
        ImGui::SliderFloat3("All Shards Added Direction", &m_AllShardsAddedDirection[0], -10.f, 10.f);
        ImGui::SliderFloat("All Shards Velocity Factor", &m_AllShardsVelocityFactor, mmin, mmax);

        ImGui::SliderFloat3("BackTopLeftDirection", &m_BaseBackTopLeftDirection[0], -1.f, 1.f);
        ImGui::SliderFloat3("DownLeftDirection   ", &m_BaseDownLeftDirection   [0], -1.f, 1.f);
        ImGui::SliderFloat3("FrontLeftDirection  ", &m_BaseFrontLeftDirection  [0], -1.f, 1.f);
        ImGui::SliderFloat3("LeftBackDirection   ", &m_BaseLeftBackDirection   [0], -1.f, 1.f);
        ImGui::SliderFloat3("LeftCenterDirection ", &m_BaseLeftCenterDirection [0], -1.f, 1.f);
        ImGui::SliderFloat3("RightBackDirection  ", &m_BaseRightBackDirection  [0], -1.f, 1.f);
        ImGui::SliderFloat3("RightDirection      ", &m_BaseRightDirection      [0], -1.f, 1.f);

        ImGui::SliderFloat("BackTopLeftVelocityFactor", &m_BaseBackTopLeftVelocityFactor, mmin, mmax);
        ImGui::SliderFloat("DownLeftVelocityFactor   ", &m_BaseDownLeftVelocityFactor   , mmin, mmax);
        ImGui::SliderFloat("FrontLeftVelocityFactor  ", &m_BaseFrontLeftVelocityFactor  , mmin, mmax);
        ImGui::SliderFloat("LeftBackVelocityFactor   ", &m_BaseLeftBackVelocityFactor   , mmin, mmax);
        ImGui::SliderFloat("LeftCenterVelocityFactor ", &m_BaseLeftCenterVelocityFactor , mmin, mmax);
        ImGui::SliderFloat("RightBackVelocityFactor  ", &m_BaseRightBackVelocityFactor  , mmin, mmax);
        ImGui::SliderFloat("RightVelocityFactor      ", &m_BaseRightVelocityFactor      , mmin, mmax);
    }

    if(ImGui::CollapsingHeader("Clear Color")) {
        if (ImGui::ColorEdit3("clearColor", &m_ClearColor[0])) {
            glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.f);
        }
    }
    if(ImGui::CollapsingHeader("Camera")) {
        switch(m_Camera.getMode()) {
        case Camera::Mode::FreeFly:
            if(ImGui::Button("Switch to LookAt Mode")) {
                m_Camera.setMode(Camera::Mode::LookAt);
            }
            ImGui::InputFloat3("Position", &m_Camera.m_FreeFlyData.m_Position[0]);
            ImGui::InputFloat3("Forward", &m_Camera.m_FreeFlyData.m_Forward[0]);
            break;
        case Camera::Mode::LookAt:
            if(ImGui::Button("Switch to FreeFly Mode")) {
                m_Camera.setMode(Camera::Mode::FreeFly);
            }
            ImGui::InputFloat3("Target", &m_Camera.m_LookAtData.m_Target[0]);
            ImGui::InputFloat3("Forward", &m_Camera.m_LookAtData.m_Forward[0]);
            ImGui::Text("Distance from target: %f", m_Camera.getDistanceFromTarget());
            break;
        }

        if(ImGui::SliderFloat("Speed", &m_CameraSpeed, 0.001f, m_CameraMaxSpeed)) {
            m_Camera.setSpeed(m_CameraSpeed);
        }
        ImGui::SliderFloat("Vertical FOV", &m_Camera.m_FovY, 0.01f, radians(179.f));
        ImGui::SliderFloat("Near", &m_Camera.m_Near, 0.0001f, 1.f);
        ImGui::SliderFloat("Far", &m_Camera.m_Far, 100.f, 10000.f);
        ImGui::SliderFloat2("Noise Factor", &m_Camera.m_NoiseFactor[0], 0.f, 42.f);
        ImGui::SliderFloat("Noise Speed", &m_Camera.m_NoiseSpeed, 0.f, 42.f);
    }

    if(ImGui::CollapsingHeader("Sprites")) {
        ImGui::Indent();
#define SPR_GUI(i) \
        ImGui::PushID(i); \
        if(ImGui::CollapsingHeader(#i + strlen("Sprites::Spr"))) { \
            ImGui::SliderFloat2("Position", &m_Sprites.m_SprPosition[i][0], -2, 2); \
            ImGui::SliderFloat("Scale", &m_Sprites.m_SprScale[i], 0, 4); \
            ImGui::SliderFloat("Alpha", &m_Sprites.m_SprAlpha[i], 0, 1); \
        } \
        ImGui::PopID();
        SPR_GUI(Sprites::SprImacLogo);
        SPR_GUI(Sprites::SprYoanLecoq);
        SPR_GUI(Sprites::SprCoralieGoldbaum);
        SPR_GUI(Sprites::SprTeacher);
        SPR_GUI(Sprites::SprSoundtrack);
        SPR_GUI(Sprites::SprRevolve);
#undef SPR_GUI
        ImGui::Unindent();
    }

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

    if(ImGui::CollapsingHeader("Skybox")) {
        ImGui::SliderFloat("Skybox scale", &m_Skybox.m_Scale, 1.f, getCurrentSceneDiagonalLength() * 2.f);
        ImGui::RadioButton("Simple Color Test", &m_Skybox.m_CurrentSky, Skybox::SkySimpleColorTest);
        ImGui::RadioButton("Space Kurt", &m_Skybox.m_CurrentSky, Skybox::SkySpaceKurt);
        ImGui::RadioButton("Space Ulukai Corona", &m_Skybox.m_CurrentSky, Skybox::SkySpaceUlukaiCorona);
        ImGui::RadioButton("Space Ulukai Red Eclipse", &m_Skybox.m_CurrentSky, Skybox::SkySpaceUlukaiRedEclipse);
		ImGui::RadioButton("Planet Flash Back", &m_Skybox.m_CurrentSky, Skybox::SkyPlanetFlashBack);
		ImGui::RadioButton("Space", &m_Skybox.m_CurrentSky, Skybox::SkySpace);
    }

    // const float sceneBoundary = getCurrentSceneDiagonalLength();

    if(ImGui::CollapsingHeader("Left Reactor Particles")) {
        ImGui::PushID(13);
        const int pcount = m_ParticlesManager.m_LeftReactorParticles.getParticleCount();
        int new_pcount = pcount;
        ImGui::InputInt("Particle Count", &new_pcount);
        if(new_pcount > pcount) {
            m_ParticlesManager.m_LeftReactorParticles.addParticles(new_pcount - pcount, m_ParticlesManager.m_LeftReactorParticlesInstanceData);
        } else if(new_pcount < pcount) {
            m_ParticlesManager.m_LeftReactorParticles.removeParticles(pcount - new_pcount);
        }
        ImGui::SliderFloat("Spawn Radius", &m_ParticlesManager.m_LeftReactorParticles.m_SpawnRadius, 0, 1);

        ImGui::SliderFloat3("Origin"       , &m_ParticlesManager.m_LeftReactorParticlesInstanceData.origin[0], -1, 1);
        ImGui::SliderFloat("Vel Multiplier", &m_ParticlesManager.m_LeftReactorParticlesInstanceData.velMultiplier, -10.f, 100.f);
        ImGui::SliderFloat("Point Size"    , &m_ParticlesManager.m_LeftReactorParticlesInstanceData.pointSize, 0, 8.f);
        ImGui::SliderFloat("Z Scale"       , &m_ParticlesManager.m_LeftReactorParticlesInstanceData.zScale, 0, 100.f);
        ImGui::SliderFloat("Z Influence"   , &m_ParticlesManager.m_LeftReactorParticlesInstanceData.zInfluence, 0, 1);
        ImGui::ColorEdit4 ("Inner Color"   , &m_ParticlesManager.m_LeftReactorParticlesInstanceData.innerColor[0]);
        ImGui::ColorEdit4 ("Outer Color"   , &m_ParticlesManager.m_LeftReactorParticlesInstanceData.outerColor[0]);
        ImGui::PopID();
    }

    if(ImGui::CollapsingHeader("Right Reactor Particles")) {
        ImGui::PushID(42);
        const int pcount = m_ParticlesManager.m_RightReactorParticles.getParticleCount();
        int new_pcount = pcount;
        ImGui::InputInt("Particle Count", &new_pcount);
        if(new_pcount > pcount) {
            m_ParticlesManager.m_RightReactorParticles.addParticles(new_pcount - pcount, m_ParticlesManager.m_RightReactorParticlesInstanceData);
        } else if(new_pcount < pcount) {
            m_ParticlesManager.m_RightReactorParticles.removeParticles(pcount - new_pcount);
        }
        ImGui::SliderFloat("Spawn Radius", &m_ParticlesManager.m_RightReactorParticles.m_SpawnRadius, 0, 1);

        ImGui::SliderFloat3("Origin"       , &m_ParticlesManager.m_RightReactorParticlesInstanceData.origin[0], -1, 1);
        ImGui::SliderFloat("Vel Multiplier", &m_ParticlesManager.m_RightReactorParticlesInstanceData.velMultiplier, -10.f, 100.f);
        ImGui::SliderFloat("Point Size"    , &m_ParticlesManager.m_RightReactorParticlesInstanceData.pointSize, 0, 8.f);
        ImGui::SliderFloat("Z Scale"       , &m_ParticlesManager.m_RightReactorParticlesInstanceData.zScale, 0, 100.f);
        ImGui::SliderFloat("Z Influence"   , &m_ParticlesManager.m_RightReactorParticlesInstanceData.zInfluence, 0, 1);
        ImGui::ColorEdit4 ("Inner Color"   , &m_ParticlesManager.m_RightReactorParticlesInstanceData.innerColor[0]);
        ImGui::ColorEdit4 ("Outer Color"   , &m_ParticlesManager.m_RightReactorParticlesInstanceData.outerColor[0]);
        ImGui::PopID();
    }

    if(ImGui::CollapsingHeader("Point Lights")) {
        ImGui::SliderInt("Count", &m_Lighting.pointLightCount, 0, GLDeferredShadingPassProgram::MAX_POINT_LIGHTS);
        ImGui::Indent();
        for(int i=0 ; i<m_Lighting.pointLightCount ; ++i) {
            char txt[32];
            snprintf(txt, sizeof txt, "Point Light n°%i", i);
            ImGui::PushID(i);
            if(ImGui::CollapsingHeader(txt)) {
                ImGui::ColorEdit3("Intensity", &m_Lighting.pointLightIntensity[i][0]);
                ImGui::InputFloat3("Position", &m_Lighting.pointLightPosition[i][0]);
                ImGui::InputFloat("Range", &m_Lighting.pointLightRange[i]);
                ImGui::SliderFloat("Attenuation Factor", &m_Lighting.pointLightAttenuationFactor[i], 0, 1.f);
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

            if(ImGui::SliderFloat3("Geom Center", &m_SceneCenterForShadowMap[0], -getCurrentSceneDiagonalLength() / 2.f, getCurrentSceneDiagonalLength() / 2.f))
                m_DirectionalShadowMapping.m_IsDirty = true;
            if(ImGui::SliderFloat("Geom Radius", &m_SceneRadiusForShadowMap, 0.01f, getCurrentSceneDiagonalLength()))
                m_DirectionalShadowMapping.m_IsDirty = true;

            ImGui::SliderFloat("Bias", &m_Lighting.dirLightShadowMapBias, 0, 0.1f);
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
			ImGui::SliderFloat("Glitch", &cpass.m_Glitch, 0, 50);
        }
    }

    ImGui::End();

    cpass.m_Time = glfwGetTime();

    const auto viewportSize = m_GLFWHandle.framebufferSize();
    glViewport(0, 0, viewportSize.x, viewportSize.y);
    ImGui::Render();
}

GLuint Demo::getHighestGeometryTextureUnit() const {
    return 16; // Totally not a hack
}

void Demo::renderGeometryForShadowMap() {
    switch(m_SceneID) {
    case SCENEID_CITY: m_City.render(); break;
    case SCENEID_END_OF_THE_WORLD:
        m_BaseBackTopLeft.render();
        m_BaseBase       .render();
        m_BaseDownLeft   .render();
        m_BaseFrontLeft  .render();
        m_BaseLeftBack   .render();
        m_BaseLeftCenter .render();
        m_BaseRightBack  .render();
        m_BaseRight      .render();
        m_Piste          .render();
        m_Ship           .render();
        // m_Sun            .render();
        break;
    }
}

void Demo::renderGeometry(const GLMaterialProgram& prog) {
    switch(m_SceneID) {
    case SCENEID_CITY:
        m_City.render(prog, m_Camera, m_CityInstanceData);
        break;
    case SCENEID_END_OF_THE_WORLD:
        m_BaseBackTopLeft.render(prog, m_Camera, m_BaseBackTopLeftInstanceData);
        m_BaseDownLeft   .render(prog, m_Camera, m_BaseDownLeftInstanceData);
        m_BaseFrontLeft  .render(prog, m_Camera, m_BaseFrontLeftInstanceData);
        m_BaseLeftBack   .render(prog, m_Camera, m_BaseLeftBackInstanceData);
        m_BaseLeftCenter .render(prog, m_Camera, m_BaseLeftCenterInstanceData);
        m_BaseRightBack  .render(prog, m_Camera, m_BaseRightBackInstanceData);
        m_BaseRight      .render(prog, m_Camera, m_BaseRightInstanceData);
        m_BaseBase       .render(prog, m_Camera, m_EndOfTheWorldInstanceData);
        m_Sun            .render(prog, m_Camera, m_EndOfTheWorldInstanceData);
        m_Piste          .render(prog, m_Camera, m_EndOfTheWorldInstanceData);
        m_Ship           .render(prog, m_Camera, m_ShipInstanceData);
        break;
    }
    m_Skybox.render(m_Camera);
    if(m_PipelineKind == PIPELINE_FORWARD) {
        m_ParticlesManager.render(m_Camera);
    }
}

void Demo::renderGeometryAfterPostFX() {
    if(m_PipelineKind == PIPELINE_FORWARD) {
        m_Sprites.render(m_nWindowWidth, m_nWindowHeight);
    }
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
    const auto& sceneCenter = m_SceneCenterForShadowMap;
    const float& sceneRadius = m_SceneRadiusForShadowMap;

    // const auto dirLightUpVector = vec3(0,1,0);
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
        renderGeometryForShadowMap();
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
        m_Lighting.dirLightShadowMap = getHighestGeometryTextureUnit();
        glActiveTexture(GL_TEXTURE0 + m_Lighting.dirLightShadowMap);
        m_DirectionalShadowMapping.m_Texture.bind();
        m_DirectionalShadowMapping.m_Sampler.bindToTextureUnit(m_Lighting.dirLightShadowMap);
        m_Lighting.dirLightViewProjMatrix = dirLightProjMatrix * dirLightViewMatrix * m_Camera.getRcpViewMatrix();
        m_ForwardRendering.m_Program.setLightingUniforms(m_Lighting, m_Camera);
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
            m_DeferredRendering.m_ShadingPassProgram.setLightingUniforms(m_Lighting, m_Camera);
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
        cpass.m_Program.setUniformGlitch(cpass.m_Glitch);
		cpass.m_Program.setUniformTime(cpass.m_Time);
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

    renderGeometryAfterPostFX();
}

void Demo::update(float dt) {
    m_Story.update(m_GLFWHandle, dt);
    auto guiHasFocus = ImGui::GetIO().WantCaptureMouse 
                    || ImGui::GetIO().WantCaptureKeyboard;
    if (!guiHasFocus || (guiHasFocus && m_Story.isPlaying())) {
        m_Camera.update(dt);
    }
    m_ParticlesManager.update(dt);

    m_BaseBackTopLeftInstanceData.m_Position += (normalize(m_BaseBackTopLeftDirection) + m_AllShardsAddedDirection) * m_BaseBackTopLeftVelocityFactor * m_AllShardsVelocityFactor * dt;
    m_BaseDownLeftInstanceData   .m_Position += (normalize(m_BaseDownLeftDirection   ) + m_AllShardsAddedDirection) * m_BaseDownLeftVelocityFactor    * m_AllShardsVelocityFactor * dt;
    m_BaseFrontLeftInstanceData  .m_Position += (normalize(m_BaseFrontLeftDirection  ) + m_AllShardsAddedDirection) * m_BaseFrontLeftVelocityFactor   * m_AllShardsVelocityFactor * dt;
    m_BaseLeftBackInstanceData   .m_Position += (normalize(m_BaseLeftBackDirection   ) + m_AllShardsAddedDirection) * m_BaseLeftBackVelocityFactor    * m_AllShardsVelocityFactor * dt;
    m_BaseLeftCenterInstanceData .m_Position += (normalize(m_BaseLeftCenterDirection ) + m_AllShardsAddedDirection) * m_BaseLeftCenterVelocityFactor  * m_AllShardsVelocityFactor * dt;
    m_BaseRightBackInstanceData  .m_Position += (normalize(m_BaseRightBackDirection  ) + m_AllShardsAddedDirection) * m_BaseRightBackVelocityFactor   * m_AllShardsVelocityFactor * dt;
    m_BaseRightInstanceData      .m_Position += (normalize(m_BaseRightDirection      ) + m_AllShardsAddedDirection) * m_BaseRightVelocityFactor       * m_AllShardsVelocityFactor * dt;

    switch(m_SceneID) {
    case SCENEID_END_OF_THE_WORLD:
        m_Lighting.pointLightIntensity[2] = m_ShipReactorStrength * vec3(255, 134, 56) / 255.f;
        m_Lighting.pointLightIntensity[3] = m_ShipReactorStrength * vec3(255, 134, 56) / 255.f;
        m_Lighting.pointLightPosition [2] = getShipLeftReactorPosition();
        m_Lighting.pointLightPosition [3] = getShipRightReactorPosition();
        break;
    case SCENEID_CITY:
        break;
    }

    m_ParticlesManager.m_LeftReactorParticlesInstanceData.origin = getShipLeftReactorPosition();
    m_ParticlesManager.m_LeftReactorParticlesInstanceData.forward = -m_ShipInstanceData.m_Forward;

    m_ParticlesManager.m_RightReactorParticlesInstanceData.origin = getShipRightReactorPosition();
    m_ParticlesManager.m_RightReactorParticlesInstanceData.forward = -m_ShipInstanceData.m_Forward;



    if(!m_Story.isPlaying())
        return;



    // FIXME(coraliegold):
    // Mettre à true pour expérimenter, puis conditionner d'après getPlayheadTime() une fois prêt
    if(false) {
        m_ParticlesManager.m_LeftReactorParticles.addParticles(2, m_ParticlesManager.m_LeftReactorParticlesInstanceData);
        m_ParticlesManager.m_RightReactorParticles.addParticles(2, m_ParticlesManager.m_RightReactorParticlesInstanceData);
    }

    // NOTE(coraliegold):
    // - Quand les réacteurs sont en marche, il faut ajouter 2 particules à chaque frame.
    //   On ne les supprime pas derrière (i.e on n'appelle jamais removeParticles(), point).
    //
    // - m_ShipReactorStrength est une variable entre 0 et 1 qui va changer l'intensité
    //   des point lights des réacteurs.
    //
    // - m_ShipInstanceData.m_Position
    // - m_ShipInstanceData.m_Forward
    // - m_AllShardsAddedDirection
    // - m_AllShardsVelocityFactor
    // - ... Sinon voir le GUI

    const auto& s = m_Story;
    const float t = s.getPlayheadTime();
    // TODO(coraliegold): For each timeline, evaluate the current value, juste like the following lines.
    /*m_Camera.setMode(s.m_CameraMode.at(t));
    m_Camera.m_LookAtData.m_Forward = s.m_CameraForward.at(t);
    m_Camera.m_FreeFlyData.m_Forward = s.m_CameraForward.at(t);
    m_Camera.m_NoiseFactor = s.m_CameraNoiseFactor.at(t);
    m_Camera.m_NoiseSpeed = s.m_CameraNoiseSpeed.at(t);
    m_Camera.m_FovY = s.m_CameraFovY.at(t);*/

	// Pipeline
	m_PipelineKind = s.m_Pipeline.at(t);

    // Scene
    changeSceneIDAndConfigure(s.m_SceneID.at(t));

	// Sprites
	m_Sprites.m_SprAlpha[1] = s.m_SpritesYoanLecoqAlpha.at(t);
	m_Sprites.m_SprAlpha[2] = s.m_SpritesCoralieGoldbaumAlpha.at(t);
	m_Sprites.m_SprAlpha[3] = s.m_SpritesTeacherAlpha.at(t);
	m_Sprites.m_SprAlpha[4] = s.m_SpritesSoundtrackAlpha.at(t);
	m_Sprites.m_SprAlpha[5] = s.m_SpritesRevolveAlpha.at(t);
	m_Sprites.m_SprAlpha[0] = s.m_SpritesIMACAlpha.at(t);
	m_Sprites.m_SprPosition[1] = s.m_SpritesYoanLecoqPos.at(t);
	m_Sprites.m_SprPosition[2] = s.m_SpritesCoralieGoldbaumPos.at(t);
	m_Sprites.m_SprPosition[3] = s.m_SpritesTeacherPos.at(t);
	m_Sprites.m_SprPosition[4] = s.m_SpritesSoundtrackPos.at(t);
	m_Sprites.m_SprScale[4] = s.m_SpritesSoundtrackScale.at(t);
	m_Sprites.m_SprScale[3] = s.m_SpritesTeacherScale.at(t);

	// Compute Pass
	m_PostFX.m_ComputePass.m_IsEnabled = s.m_ComputePass.at(t);
	m_PostFX.m_ComputePass.m_Gamma = s.m_ComputePassGamma.at(t);
	m_PostFX.m_ComputePass.m_FinalTouchAdd = s.m_ComputePassFinalTouchAdd.at(t);
	m_PostFX.m_ComputePass.m_FinalTouchMul = s.m_ComputePassFinalTouchMul.at(t);

	// Fragment Pass
	m_PostFX.m_FragmentPass.m_IsEnabled = s.m_FragmentPass.at(t);
	m_PostFX.m_FragmentPass.m_BlurTechnique = s.m_BlurKind.at(t);
	m_PostFX.m_FragmentPass.m_BloomEnabled = s.m_Bloom.at(t);
	m_PostFX.m_FragmentPass.m_BloomMatrixHalfSide = s.m_BloomHalfSide.at(t);
	m_PostFX.m_FragmentPass.m_BloomTexelSkip = s.m_BloomTexelSkip.at(t);
	m_PostFX.m_FragmentPass.m_BloomThreshold = s.m_BloomThreshold.at(t);

	// Camera
    m_Camera.setSpeed(m_CameraSpeed);
	m_Camera.setMode(s.m_CameraMode.at(t));
	m_Camera.m_LookAtData.m_Forward = s.m_CameraForward.at(t);
	m_Camera.m_LookAtData.m_Target = s.m_CameraTarget.at(t);
	m_Camera.m_FovY = s.m_CameraVerticalFOV.at(t);
	m_Camera.m_NoiseFactor = s.m_CameraNoiseFactor.at(t);
	m_Camera.m_NoiseSpeed = s.m_CameraNoiseSpeed.at(t);
	m_Camera.m_FreeFlyData.m_Forward = s.m_CameraFreeflyForward.at(t);
	m_Camera.m_FreeFlyData.m_Position = s.m_CameraFreeflyPosition.at(t);

	// Skybox
	m_Skybox.m_CurrentSky = s.m_Skybox.at(t);
}

int Demo::run() {
    for (auto frameCount = 0u; !m_GLFWHandle.shouldClose(); ++frameCount) {
        const auto seconds = glfwGetTime();
        renderFrame();
        renderGUI();
        glfwPollEvents();
        m_GLFWHandle.swapBuffers();
        update(glfwGetTime() - seconds);
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
    m_SceneID(SCENEID_END_OF_THE_WORLD),

    m_BaseBackTopLeft(m_Paths.m_AssetsRoot / "demo" / "models" / "end_of_the_world_2" / "BaseBackTopLeft.obj"),
    m_BaseBase       (m_Paths.m_AssetsRoot / "demo" / "models" / "end_of_the_world_2" / "BaseBase.obj"),
    m_BaseDownLeft   (m_Paths.m_AssetsRoot / "demo" / "models" / "end_of_the_world_2" / "BaseDownLeft.obj"),
    m_BaseFrontLeft  (m_Paths.m_AssetsRoot / "demo" / "models" / "end_of_the_world_2" / "BaseFrontLeft.obj"),
    m_BaseLeftBack   (m_Paths.m_AssetsRoot / "demo" / "models" / "end_of_the_world_2" / "BaseLeftBack.obj"),
    m_BaseLeftCenter (m_Paths.m_AssetsRoot / "demo" / "models" / "end_of_the_world_2" / "BaseLeftCenter.obj"),
    m_BaseRightBack  (m_Paths.m_AssetsRoot / "demo" / "models" / "end_of_the_world_2" / "BaseRightBack.obj"),
    m_BaseRight      (m_Paths.m_AssetsRoot / "demo" / "models" / "end_of_the_world_2" / "BaseRight.obj"),
    m_Piste          (m_Paths.m_AssetsRoot / "demo" / "models" / "end_of_the_world_2" / "Piste.obj"),
    m_Ship           (m_Paths.m_AssetsRoot / "demo" / "models" / "end_of_the_world_2" / "Ship.obj"),
    m_Sun            (m_Paths.m_AssetsRoot / "demo" / "models" / "end_of_the_world_2" / "Sun.obj"),

	m_EndOfTheWorldInstanceData(),
	m_ShipInstanceData(),
    m_BaseBackTopLeftInstanceData(),
    m_BaseDownLeftInstanceData   (),
    m_BaseFrontLeftInstanceData  (),
    m_BaseLeftBackInstanceData   (),
    m_BaseLeftCenterInstanceData (),
    m_BaseRightBackInstanceData  (),
    m_BaseRightInstanceData      (),

    m_AllShardsAddedDirection(0),
    m_BaseBackTopLeftDirection(-1,0,0),
    m_BaseDownLeftDirection   (-1,-0.3,0.7),
    m_BaseFrontLeftDirection  (-1,0,-0.7),
    m_BaseLeftBackDirection   (-0.2,0,1),
    m_BaseLeftCenterDirection (-1,0,0.7),
    m_BaseRightBackDirection  (0.3,0,1),
    m_BaseRightDirection      (1,0,0.2),

    m_AllShardsVelocityFactor(0),
    m_BaseBackTopLeftVelocityFactor(1),
    m_BaseDownLeftVelocityFactor   (1),
    m_BaseFrontLeftVelocityFactor  (1),
    m_BaseLeftBackVelocityFactor   (1),
    m_BaseLeftCenterVelocityFactor (1),
    m_BaseRightBackVelocityFactor  (1),
    m_BaseRightVelocityFactor      (1),

    m_ShipReactorStrength(0),

	m_City(m_Paths.m_AssetsRoot / "demo" / "models" / "city" / "city.obj"),
	m_CityInstanceData(),

    m_SceneCenterForShadowMap(0),
    m_SceneRadiusForShadowMap(m_Sun.getDiagonalLength() / 2.f),

    m_Sprites(m_Paths),
    m_Camera(m_GLFWHandle.window(), m_nWindowWidth, m_nWindowHeight),
    m_CameraMaxSpeed(m_Sun.getDiagonalLength() / 2.f),
    m_CameraSpeed(m_CameraMaxSpeed / 5.f),
    m_Skybox(m_Paths, m_Sun.getDiagonalLength() / 2.f),
    m_ParticlesManager(m_Paths),
    m_Story(m_Paths)
{
    (void) argc;
    static_ImGuiIniFilename = m_Paths.m_AppName + ".imgui.ini";
    ImGui::GetIO().IniFilename = static_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    // glEnable(GL_CULL_FACE);
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

    // Force update
    m_SceneID = -1;
    changeSceneIDAndConfigure(SCENEID_END_OF_THE_WORLD);
}
