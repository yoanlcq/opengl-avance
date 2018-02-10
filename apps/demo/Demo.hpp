#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLDeferredGPassProgram.hpp>
#include <glmlv/GLDeferredShadingPassProgram.hpp>
#include <glmlv/GLDirectionalSMProgram.hpp>
#include <glmlv/GLDisplayDepthMapProgram.hpp>
#include <glmlv/GLGammaCorrectProgram.hpp>
#include <glmlv/GLTexture2D.hpp>
#include <glmlv/GLSampler.hpp>
#include <glmlv/Scene.hpp>
#include <glmlv/Camera.hpp>


void handleFboStatus(GLenum status);

enum GBufferTexIndex
{
    GPosition = 0,
    GNormal,
    GAmbient,
    GDiffuse,
    GGlossyShininess,
    GDepth, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
    GBufferTextureCount
};

struct Paths {
    const glmlv::fs::path m_App;
    const std::string     m_AppName;
    const glmlv::fs::path m_AssetsRoot, m_ShadersRoot;
    const glmlv::fs::path m_AppAssets, m_AppShaders;

    Paths(glmlv::fs::path&& appPath):
        m_App(appPath),
        m_AppName     { m_App.stem().string() },
        m_AssetsRoot  { m_App.parent_path() / "assets" },
        m_ShadersRoot { m_App.parent_path() / "shaders" },
        m_AppAssets   { m_AssetsRoot / m_AppName },
        m_AppShaders  { m_ShadersRoot / m_AppName }
        {}
};

struct Deferred {
    const glmlv::GLDeferredGPassProgram       m_GPassProgram;
    const glmlv::GLDeferredShadingPassProgram m_ShadingPassProgram;
    glmlv::GLTexture2D                  m_GBufferTextures[GBufferTextureCount];
    GLuint                                    m_GBufferFbo;
    glmlv::GLDeferredShadingPassProgram::LightingUniforms m_Lighting;

    bool m_GuiDisplaysGBufferTextures;
    int m_GuiGBufferTexIndex;

    Deferred(const Paths& paths, GLsizei w, GLsizei h):
        m_GPassProgram(
            paths.m_AppShaders / "geometryPass.vs.glsl",
            paths.m_AppShaders / "geometryPass.fs.glsl"
        ),
        m_ShadingPassProgram(
            paths.m_AppShaders / "shadingPass.vs.glsl",
            paths.m_AppShaders / "shadingPass.fs.glsl"
        ),
        m_GBufferTextures {
            { GL_RGB32F, w, h },
            { GL_RGB32F, w, h },
            { GL_RGB32F, w, h },
            { GL_RGB32F, w, h },
            { GL_RGBA32F, w, h },
            { GL_DEPTH_COMPONENT32F, w, h }
        },
        m_GBufferFbo(0),
        m_Lighting(),
        m_GuiDisplaysGBufferTextures(false),
        m_GuiGBufferTexIndex(GNormal)
    {
        glGenFramebuffers(1, &m_GBufferFbo);
        assert(m_GBufferFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_GBufferFbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBufferTextures[0].glId(), 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBufferTextures[1].glId(), 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBufferTextures[2].glId(), 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_GBufferTextures[3].glId(), 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_GBufferTextures[4].glId(), 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, m_GBufferTextures[5].glId(), 0);
        static const GLenum drawBuffers[5] = {
            GL_COLOR_ATTACHMENT0, 
            GL_COLOR_ATTACHMENT1, 
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4
        };
        glDrawBuffers(5, drawBuffers);
        handleFboStatus(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
};

struct ShadowMapping {
    int m_Resolution; // NOTE: int to be editable via RadioButton
    const glmlv::GLDirectionalSMProgram m_Program;
    const glmlv::GLDisplayDepthMapProgram m_DisplayDepthMapProgram;
    const glmlv::GLSampler m_Sampler;
    glmlv::GLTexture2D m_Texture;
    GLuint m_Fbo;

    bool m_IsDirty;
    bool m_GuiDisplaysShadowMap;

    ShadowMapping(const Paths& paths): 
        m_Resolution(512),
        m_Program(
            paths.m_AppShaders / "directionalSM.vs.glsl",
            paths.m_AppShaders / "directionalSM.fs.glsl"
        ),
        m_DisplayDepthMapProgram(
            paths.m_AppShaders / "shadingPass.vs.glsl",
            paths.m_AppShaders / "displayDepth.fs.glsl"
        ),
        m_Sampler(glmlv::GLSamplerParams().withWrapST(GL_CLAMP_TO_BORDER).withMinMagFilter(GL_LINEAR)),
        m_Texture(GL_DEPTH_COMPONENT32F, m_Resolution, m_Resolution),
        m_Fbo(0),
        m_IsDirty(true),
        m_GuiDisplaysShadowMap(false)
    {
        glSamplerParameteri(m_Sampler.glId(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glSamplerParameteri(m_Sampler.glId(), GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glGenFramebuffers(1, &m_Fbo);
        assert(m_Fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_Texture.glId(), 0);
        handleFboStatus(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
};


struct PostFX_TextureFbo {
    const glmlv::GLTexture2D m_Texture;
    GLuint m_Fbo;

    PostFX_TextureFbo(GLsizei w, GLsizei h): 
        m_Texture(GL_RGBA32F, w, h),
        m_Fbo(0)
    {
        glGenFramebuffers(1, &m_Fbo);
        assert(m_Fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture.glId(), 0);
        static const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBuffers);
        handleFboStatus(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
};

struct PostFX {
    const glmlv::GLGammaCorrectProgram m_Program;
    const PostFX_TextureFbo m_Input, m_Output;
    bool m_IsEnabled;
    float m_Gamma;

    PostFX(const Paths& paths, GLsizei w, GLsizei h):
        m_Program(paths.m_AppShaders / "gammaCorrect.cs.glsl"),
        m_Input(w, h),
        m_Output(w, h),
        m_IsEnabled(true),
        m_Gamma(2.2f)
    {}
};

class Demo {
public:
    Demo(int argc, char** argv);

    int run();
private:
    // NOTE: Make it static, so that the char pointer's lifetime is unbounded.
    // With the old code, the memory was freed before ImGUI wrote to the ini filename.
    static std::string static_ImGuiIniFilename;

    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Creating Dimensions" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. glmlv::GLProgram, GLShader)

    Paths m_Paths;
    Deferred m_Deferred;
    ShadowMapping m_ShadowMapping;
    PostFX m_PostFX;

    glm::vec3 m_ClearColor;
    float m_DirLightPhiAngleDegrees; // Angle around Y
    float m_DirLightThetaAngleDegrees; // Angle around X
    glmlv::Camera m_Camera;
    glmlv::Scene m_Sponza;
    glmlv::SceneInstanceData m_SponzaInstanceData;
    glmlv::GLMesh m_ScreenCoverQuad;
};
