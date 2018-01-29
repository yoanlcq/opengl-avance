#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/simple_geometry.hpp>

using glmlv::SimpleGeometry;
using glmlv::Vertex3f3f2f;

struct GLMesh {
    GLuint vao, vbo, ibo;
    GLsizei elementCount;
    ~GLMesh() {
        glDeleteBuffers(1, &ibo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
    GLMesh() = delete;
    GLMesh(const SimpleGeometry& mesh) {
        elementCount = mesh.indexBuffer.size();
        vao = 0;
        vbo = 0;
        ibo = 0;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);
        assert(vao);
        assert(vbo);
        assert(ibo);
        auto& v = mesh.vertexBuffer;
        auto& i = mesh.indexBuffer;
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof v[0], v.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, i.size() * sizeof i[0], i.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        auto stride = sizeof mesh.vertexBuffer[0];
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex3f3f2f, position));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex3f3f2f, normal));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex3f3f2f, texCoords));
        static_assert(sizeof mesh.indexBuffer[0] == sizeof(uint32_t), "We are expecting to use GL_UNSIGNED_INT for indices!");
    }
    GLMesh(const GLMesh& v) = delete;
    GLMesh& operator=(const GLMesh& v) = delete;
    GLMesh(GLMesh&& o): vao(o.vao), vbo(o.vbo), ibo(o.ibo), elementCount(o.elementCount) {
        o.vao = o.vbo = o.ibo = 0;
    }
    GLMesh& operator=(GLMesh&& o) {
        std::swap(vao, o.vao);
        std::swap(vbo, o.vbo);
        std::swap(ibo, o.ibo);
        std::swap(elementCount, o.elementCount);
    }
    void render() const {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, nullptr);
    }
};

struct Mesh {
    SimpleGeometry mesh;
    GLMesh gl;
    Mesh() = delete;
    Mesh(SimpleGeometry mesh) : mesh(mesh), gl(mesh) {}
    void render() const {
        gl.render();
    }
};


class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_ForwardVsPath;
    const glmlv::fs::path m_ForwardFsPath;
    const glmlv::GLProgram m_ForwardProgram;
    const GLint m_UniformModelViewProjMatrixLocation;
    const GLint m_UniformModelViewMatrixLocation;
    const GLint m_UniformNormalMatrixLocation;
    const Mesh m_Cube, m_Sphere;
};
