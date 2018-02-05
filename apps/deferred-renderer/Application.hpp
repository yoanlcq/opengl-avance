#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLDeferredGPassProgram.hpp>
#include <glmlv/GLDeferredShadingPassProgram.hpp>
#include <glmlv/GLDirectionalSMProgram.hpp>
#include <glmlv/GLDisplayDepthMapProgram.hpp>
#include <glmlv/GLTexture2D.hpp>
#include <glmlv/GLSampler.hpp>
#include <glmlv/Scene.hpp>
#include <glmlv/Camera.hpp>

enum GBufferTextureType
{
    GPosition = 0,
    GNormal,
    GAmbient,
    GDiffuse,
    GGlossyShininess,
    GDepth, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
    GBufferTextureCount
};

class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Deferred Rendering" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. glmlv::GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const glmlv::fs::path m_AssetsRootPath;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::Scene m_Scene;
    glmlv::Camera m_ViewController;
    const glmlv::GLDeferredGPassProgram m_DeferredGPassProgram;
    const glmlv::GLDeferredShadingPassProgram m_DeferredShadingPassProgram;
    const glmlv::GLDirectionalSMProgram m_DirectionalSMProgram;
    const glmlv::GLDisplayDepthMapProgram m_DisplayDepthMapProgram;
    glmlv::GLTexture2D m_GBufferTextures[GBufferTextureCount];
    static const GLenum static_GBufferTextureFormat[GBufferTextureCount];
    GLuint m_Fbo;
    const glmlv::GLTexture2D m_directionalSMTexture;
    GLuint m_directionalSMFBO;
    const glmlv::GLSampler m_directionalSMSampler;
    static const size_t static_nDirectionalSMResolution = 512;
    // NOTE: Make it static, so that the char pointer's lifetime is unbounded.
    // With the old code, the memory was freed before ImGUI wrote to the ini filename.
    static std::string static_ImGuiIniFilename;
};
