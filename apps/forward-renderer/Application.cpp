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
    vec3 dirLightDir(1,0,0);
    vec3 dirLightIntensity(1,1,1);
    vec3 pointLightDir(0,0,1);
    vec3 pointLightIntensity(1,1,1);
    vec3 cubeColor(1,1,1);
    vec3 sphereColor(1,1,1);

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_ForwardProgram.use();

        glUniform3fv(m_UniformDirectionalLightDirLocation, 1, &dirLightDir[0]);
        glUniform3fv(m_UniformDirectionalLightIntensityLocation, 1, &dirLightIntensity[0]);
        glUniform3fv(m_UniformPointLightPositionLocation, 1, &pointLightDir[0]);
        glUniform3fv(m_UniformPointLightIntensityLocation, 1, &pointLightIntensity[0]);

        const float fovy = radians(60.f), near = 0.5, far = 5000, aspect = m_nWindowWidth / float(m_nWindowHeight);
        mat4 proj(perspective(fovy, aspect, near, far));
        mat4 view = m_ViewController.getViewMatrix();

        const auto sceneDiagonalSize = glm::length(m_Scene.objData.bboxMax - m_Scene.objData.bboxMin);
        m_ViewController.setSpeed(sceneDiagonalSize * 0.1f); // 10% de la scene parcouru par seconde

        // Render the sphere
        {
            mat4 model(translate(mat4(1), vec3(0,0,-2)));
            mat4 modelView(view * model);
            mat4 modelViewProj(proj * modelView);
            mat4 normalMatrix(transpose(inverse(modelView)));

            const GLuint unit = 0;
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, m_SphereTex.texid);
            glBindSampler(unit, m_SphereTex.sampler);

            glUniform1i(m_UniformKdSamplerLocation, unit);
            glUniform3fv(m_UniformKdLocation, 1, &sphereColor[0]);
            glUniformMatrix4fv(m_UniformModelViewProjMatrixLocation, 1, GL_FALSE, &modelViewProj[0][0]);
            glUniformMatrix4fv(m_UniformModelViewMatrixLocation, 1, GL_FALSE, &modelView[0][0]);
            glUniformMatrix4fv(m_UniformNormalMatrixLocation, 1, GL_FALSE, &normalMatrix[0][0]);
            
            m_Sphere.render();
        }

        // Render the cube
        {
            mat4 model(translate(mat4(1), vec3(-2,0,-2)));
            mat4 modelView(view * model);
            mat4 modelViewProj(proj * modelView);
            mat4 normalMatrix(transpose(inverse(modelView)));

            const GLuint unit = 1;
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, m_CubeTex.texid);
            glBindSampler(unit, m_CubeTex.sampler);

            glUniform1i(m_UniformKdSamplerLocation, unit);
            glUniform3fv(m_UniformKdLocation, 1, &cubeColor[0]);
            glUniformMatrix4fv(m_UniformModelViewProjMatrixLocation, 1, GL_FALSE, &modelViewProj[0][0]);
            glUniformMatrix4fv(m_UniformModelViewMatrixLocation, 1, GL_FALSE, &modelView[0][0]);
            glUniformMatrix4fv(m_UniformNormalMatrixLocation, 1, GL_FALSE, &normalMatrix[0][0]);

            m_Cube.render();
        }

        // Render the loaded OBJ scene
        {
            mat4 model(translate(mat4(1), vec3(2,0,-2)));
            mat4 modelView(view * model);
            mat4 modelViewProj(proj * modelView);
            mat4 normalMatrix(transpose(inverse(modelView)));

            const GLuint unit = 1;
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, m_CubeTex.texid);
            glBindSampler(unit, m_CubeTex.sampler);

            glUniform1i(m_UniformKdSamplerLocation, unit);
            glUniform3fv(m_UniformKdLocation, 1, &cubeColor[0]);
            glUniformMatrix4fv(m_UniformModelViewProjMatrixLocation, 1, GL_FALSE, &modelViewProj[0][0]);
            glUniformMatrix4fv(m_UniformModelViewMatrixLocation, 1, GL_FALSE, &modelView[0][0]);
            glUniformMatrix4fv(m_UniformNormalMatrixLocation, 1, GL_FALSE, &normalMatrix[0][0]);

            m_Scene.render();
        }

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
            EDIT_DIRECTION(dirLightDir);
            EDIT_DIRECTION(pointLightDir);
#undef EDIT_DIRECTION
#define EDIT_COLOR(c) ImGui::ColorEdit3(#c, &c[0])
            EDIT_COLOR(dirLightIntensity);
            EDIT_COLOR(pointLightIntensity);
            EDIT_COLOR(cubeColor);
            EDIT_COLOR(sphereColor);
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
    m_ForwardProgram(glmlv::compileProgram({ m_ForwardVsPath.string(), m_ForwardFsPath.string() })),
    m_UniformModelViewProjMatrixLocation      (m_ForwardProgram.getUniformLocation("uModelViewProjMatrix")),
    m_UniformModelViewMatrixLocation          (m_ForwardProgram.getUniformLocation("uModelViewMatrix")),
    m_UniformNormalMatrixLocation             (m_ForwardProgram.getUniformLocation("uNormalMatrix")),
    m_UniformDirectionalLightDirLocation      (m_ForwardProgram.getUniformLocation("uDirectionalLightDir")),
    m_UniformDirectionalLightIntensityLocation(m_ForwardProgram.getUniformLocation("uDirectionalLightIntensity")),
    m_UniformPointLightPositionLocation       (m_ForwardProgram.getUniformLocation("uPointLightPosition")),
    m_UniformPointLightIntensityLocation      (m_ForwardProgram.getUniformLocation("uPointLightIntensity")),
    m_UniformKdLocation                       (m_ForwardProgram.getUniformLocation("uKd")),
    m_UniformKdSamplerLocation                (m_ForwardProgram.getUniformLocation("uKdSampler")),
    // Let's pretend the below two are different files for now (the code was planned for two files anyway)
    m_CubeTex  (m_AssetsRootPath / m_AppName / "textures" / "plasma.png"),
    m_SphereTex(m_AssetsRootPath / m_AppName / "textures" / "plasma.png"),
    m_Cube(glmlv::makeCube()),
    m_Sphere(glmlv::makeSphere(32)),
    m_ViewController(m_GLFWHandle.window()),
    m_Scene(m_AssetsRootPath / "glmlv" / "models" / "crytek-sponza" / "sponza.obj")
{
    ImGui::GetIO().IniFilename = strdup(m_ImGuiIniFilename.c_str()); // At exit, ImGUI will store its windows positions in this file

#define CHECK_UNIFORM(u) if(u == -1) { std::cerr << "Warning: " << #u << "equals -1 (is it unused?)" << std::endl; }
    CHECK_UNIFORM(m_UniformModelViewProjMatrixLocation);
    CHECK_UNIFORM(m_UniformModelViewMatrixLocation);
    CHECK_UNIFORM(m_UniformNormalMatrixLocation);
    CHECK_UNIFORM(m_UniformDirectionalLightDirLocation);
    CHECK_UNIFORM(m_UniformDirectionalLightIntensityLocation);
    CHECK_UNIFORM(m_UniformPointLightPositionLocation);
    CHECK_UNIFORM(m_UniformPointLightIntensityLocation);
    CHECK_UNIFORM(m_UniformKdLocation);
    CHECK_UNIFORM(m_UniformKdSamplerLocation);
#undef CHECK_UNIFORM

    glEnable(GL_DEPTH_TEST);
}
