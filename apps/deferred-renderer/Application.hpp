#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLDeferredGPassProgram.hpp>
#include <glmlv/GLDeferredShadingPassProgram.hpp>
#include <glmlv/GLTexture2D.hpp>
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
    const glmlv::GLDeferredGPassProgram m_DeferredGPassProgram;
    const glmlv::GLDeferredShadingPassProgram m_DeferredShadingPassProgram;
    const glmlv::Scene m_Scene;
    glmlv::Camera m_ViewController;
    glmlv::GLTexture2D m_GBufferTextures[GBufferTextureCount];
    GLuint m_Fbo;
    static const GLenum static_GBufferTextureFormat[GBufferTextureCount];
    // NOTE: Make it static, so that the char pointer's lifetime is unbounded.
    // With the old code, the memory was freed before ImGUI wrote to the ini filename.
    static std::string static_ImGuiIniFilename;
};
