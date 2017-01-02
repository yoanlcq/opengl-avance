#include <iostream>
#include <experimental/filesystem>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/gl_debug_output.hpp>

namespace fs = std::experimental::filesystem;

class Application
{
public:
    Application(int argc, char** argv);

    ~Application();

    int run();
private:
    fs::path m_AppPath;
    std::string m_AppName;
    std::string m_ImGuiIniFilename;
    fs::path m_ShadersRootPath;

    size_t m_nWindowWidth = 1280;
    size_t m_nWindowHeight = 720;

    GLFWwindow* m_pWindow = nullptr;
};

int main(int argc, char** argv)
{
    Application app(argc, argv);
    return app.run();
}

Application::Application(int argc, char** argv)
{
    m_AppPath = fs::path{ argv[0] };
    m_AppName = m_AppPath.stem().string();
    auto appDir = m_AppPath.parent_path();

    if (!glfwInit()) {
        std::cerr << "Unable to init GLFW.\n";
        throw std::runtime_error("Unable to init GLFW.\n");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    m_pWindow = glfwCreateWindow(int(m_nWindowWidth), int(m_nWindowHeight), "OpenGL", NULL, NULL);
    if (!m_pWindow) {
        std::cerr << "Unable to open window.\n";
        glfwTerminate();
        throw std::runtime_error("Unable to open window.\n");
    }

    glfwMakeContextCurrent(m_pWindow);

    glfwSwapInterval(0);

    if (!gladLoadGL()) {
        std::cerr << "Unable to init OpenGL.\n";
        throw std::runtime_error("Unable to init OpenGL.\n");
    }

    glmlv::initGLDebugOutput();

    m_ShadersRootPath = appDir / "glsl";
    //loadShaders(m_ShadersRootPath, m_ShaderLibrary);

    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(m_pWindow, true);
    m_ImGuiIniFilename = m_AppName + ".imgui.ini";
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str();
}

Application::~Application()
{
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();
}

void showGLDebugOutputsWindow();

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !glfwWindowShouldClose(m_pWindow); ++iterationCount)
    {
        auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);

        // Put here rendering code

        //
        //
        //

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

        glmlv::showGLDebugOutputsWindow();

        int displayWidth, displayHeight;
        glfwGetFramebufferSize(m_pWindow, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        ImGui::Render();

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        glfwSwapBuffers(m_pWindow);

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            //viewController.update(float(ellapsedTime))
        }
    }

    return 0;
}
