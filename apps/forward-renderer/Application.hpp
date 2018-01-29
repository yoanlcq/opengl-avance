#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/load_obj.hpp>

using glmlv::SimpleGeometry;
using glmlv::Vertex3f3f2f;
using glmlv::ObjData;

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
        const auto& v = mesh.vertexBuffer;
        const auto& i = mesh.indexBuffer;
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof v[0], v.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, i.size() * sizeof i[0], i.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        const auto stride = sizeof mesh.vertexBuffer[0];
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

/*
struct ObjData
{
    size_t shapeCount; // Nombre d'objets dans l'OBJ
    size_t materialCount; // Nombre de matériaux
    // Point min et max de la bounding box de l'OBJ:
    glm::vec3 bboxMin;
    glm::vec3 bboxMax;
    std::vector<Vertex3f3f2f> vertexBuffer; // Tableau de sommets
    std::vector<uint32_t> indexBuffer; // Tableau d'indices
    std::vector<uint32_t> indexCountPerShape; // Nombre d'indices par objet

    std::vector<int32_t> materialIDPerShape; // Index de materiaux de chaque objet (pointe dans le tableau materials)

    std::vector<PhongMaterial> materials; // Tableau des materiaux
    std::vector<Image2DRGBA> textures; // Tableau dex textures référencées par les materiaux
}
*/

struct Scene {
    ObjData objData;
    GLMesh gl;
    Scene() = delete;
    Scene(const glmlv::fs::path& path) :
        objData(objDataFromPath(path)),
        gl(SimpleGeometry { objData.vertexBuffer, objData.indexBuffer })
        {}
    void render() const {
        glBindVertexArray(gl.vao);
        auto indexOffset = 0;
        for (const auto indexCount: objData.indexCountPerShape) {
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*) (indexOffset * sizeof(GLuint)));
            indexOffset += indexCount;
        };
    }
private:
    static ObjData objDataFromPath(const glmlv::fs::path& path) {
        ObjData d;
        glmlv::loadObj(path, d);
        return d;
    }
};

struct GLTexture2D {
    GLuint texid, sampler;
    GLTexture2D(const glmlv::fs::path& path): GLTexture2D(glmlv::readImage(path)) {}
    GLTexture2D(const glmlv::Image2DRGBA& img) {
        texid = 0;
        sampler = 0;
        glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);
        glTexStorage2D(
            GL_TEXTURE_2D, 1, GL_RGBA8,
            img.width(), img.height()
        );
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, img.width(), img.height(), GL_RGBA,
            GL_UNSIGNED_BYTE, img.data()
        );
        glGenSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Forward Rendering" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_AssetsRootPath;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_ForwardVsPath;
    const glmlv::fs::path m_ForwardFsPath;
    const glmlv::GLProgram m_ForwardProgram;
    const GLint m_UniformModelViewProjMatrixLocation;
    const GLint m_UniformModelViewMatrixLocation;
    const GLint m_UniformNormalMatrixLocation;
    const GLint m_UniformDirectionalLightDirLocation;
    const GLint m_UniformDirectionalLightIntensityLocation;
    const GLint m_UniformPointLightPositionLocation;
    const GLint m_UniformPointLightIntensityLocation;
    const GLint m_UniformKdLocation;
    const GLint m_UniformKdSamplerLocation;
    const GLTexture2D m_CubeTex, m_SphereTex;
    const Mesh m_Cube, m_Sphere;
    const Scene m_Scene;
    glmlv::ViewController m_ViewController;
};
