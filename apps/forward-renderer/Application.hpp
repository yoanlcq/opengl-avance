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
using glmlv::GLProgram;
using glm::mat4;
using glm::vec3;

// TODO: Move utilities to glmlv;

class GLForwardRenderingProgram: public GLProgram {
    const GLint m_UniformModelViewProjMatrixLocation       = -1;
    const GLint m_UniformModelViewMatrixLocation           = -1;
    const GLint m_UniformNormalMatrixLocation              = -1;
    const GLint m_UniformDirectionalLightDirLocation       = -1;
    const GLint m_UniformDirectionalLightIntensityLocation = -1;
    const GLint m_UniformPointLightPositionLocation        = -1;
    const GLint m_UniformPointLightIntensityLocation       = -1;
    const GLint m_UniformKaLocation                        = -1;
    const GLint m_UniformKdLocation                        = -1;
    const GLint m_UniformKsLocation                        = -1;
    const GLint m_UniformShininessLocation                 = -1;
    const GLint m_UniformKaSamplerLocation                 = -1;
    const GLint m_UniformKdSamplerLocation                 = -1;
    const GLint m_UniformKsSamplerLocation                 = -1;
    const GLint m_UniformShininessSamplerLocation          = -1;
    const GLint m_UniformKaSamplerFactorLocation           = -1;
    const GLint m_UniformKdSamplerFactorLocation           = -1;
    const GLint m_UniformKsSamplerFactorLocation           = -1;
    const GLint m_UniformShininessSamplerFactorLocation    = -1;

public:
    GLForwardRenderingProgram(const glmlv::fs::path& vs, const glmlv::fs::path& fs):
        GLProgram(glmlv::compileProgram({ vs.string(), fs.string() })),
        m_UniformModelViewProjMatrixLocation      (getUniformLocation("uModelViewProjMatrix")),
        m_UniformModelViewMatrixLocation          (getUniformLocation("uModelViewMatrix")),
        m_UniformNormalMatrixLocation             (getUniformLocation("uNormalMatrix")),
        m_UniformDirectionalLightDirLocation      (getUniformLocation("uDirectionalLightDir")),
        m_UniformDirectionalLightIntensityLocation(getUniformLocation("uDirectionalLightIntensity")),
        m_UniformPointLightPositionLocation       (getUniformLocation("uPointLightPosition")),
        m_UniformPointLightIntensityLocation      (getUniformLocation("uPointLightIntensity")),
        m_UniformKaLocation                       (getUniformLocation("uKa")),
        m_UniformKdLocation                       (getUniformLocation("uKd")),
        m_UniformKsLocation                       (getUniformLocation("uKs")),
        m_UniformShininessLocation                (getUniformLocation("uShininess")),
        m_UniformKaSamplerLocation                (getUniformLocation("uKaSampler")),
        m_UniformKdSamplerLocation                (getUniformLocation("uKdSampler")),
        m_UniformKsSamplerLocation                (getUniformLocation("uKsSampler")),
        m_UniformShininessSamplerLocation         (getUniformLocation("uShininessSampler")),
        m_UniformKaSamplerFactorLocation          (getUniformLocation("uKaSamplerFactor")),
        m_UniformKdSamplerFactorLocation          (getUniformLocation("uKdSamplerFactor")),
        m_UniformKsSamplerFactorLocation          (getUniformLocation("uKsSamplerFactor")),
        m_UniformShininessSamplerFactorLocation   (getUniformLocation("uShininessSamplerFactor"))
        {}

    // Uniform values that are not per-object-instance.
    struct SharedUniformData {
        vec3 dirLightDir = vec3(1,0,0);
        vec3 dirLightIntensity = vec3(1,1,1);
        vec3 pointLightPosition = vec3(0,0,1);
        vec3 pointLightIntensity = vec3(1,1,1);
    };

    void setSharedUniformData(const SharedUniformData& d) const {
        setUniformDirectionalLightDir(d.dirLightDir);
        setUniformDirectionalLightIntensity(d.dirLightIntensity);
        setUniformPointLightPosition(d.pointLightPosition);
        setUniformPointLightIntensity(d.pointLightIntensity);
    }

    void setUniformModelViewProjMatrix(const mat4& m) const {
        glUniformMatrix4fv(m_UniformModelViewProjMatrixLocation, 1, GL_FALSE, &m[0][0]);
    }
    void setUniformModelViewMatrix(const mat4& m) const {
        glUniformMatrix4fv(m_UniformModelViewMatrixLocation, 1, GL_FALSE, &m[0][0]);
    }
    void setUniformNormalMatrix(const mat4& m) const {
        glUniformMatrix4fv(m_UniformNormalMatrixLocation, 1, GL_FALSE, &m[0][0]);
    }
    void setUniformDirectionalLightDir(const vec3& v) const {
        glUniform3fv(m_UniformDirectionalLightDirLocation, 1, &v[0]);
    }
    void setUniformDirectionalLightIntensity(const vec3& v) const {
        glUniform3fv(m_UniformDirectionalLightIntensityLocation, 1, &v[0]);
    }
    void setUniformPointLightPosition(const vec3& v) const {
        glUniform3fv(m_UniformPointLightPositionLocation, 1, &v[0]);
    }
    void setUniformPointLightIntensity(const vec3& v) const {
        glUniform3fv(m_UniformPointLightIntensityLocation, 1, &v[0]);
    }
    void setUniformKa(const vec3& v) const { glUniform3fv(m_UniformKaLocation, 1, &v[0]); }
    void setUniformKd(const vec3& v) const { glUniform3fv(m_UniformKdLocation, 1, &v[0]); }
    void setUniformKs(const vec3& v) const { glUniform3fv(m_UniformKsLocation, 1, &v[0]); }
    void setUniformShininess(GLfloat v) const { glUniform1f(m_UniformShininessLocation, v); }
    void setUniformKaSampler(GLuint unit) const { glUniform1i(m_UniformKaSamplerLocation, unit); }
    void setUniformKdSampler(GLuint unit) const { glUniform1i(m_UniformKdSamplerLocation, unit); }
    void setUniformKsSampler(GLuint unit) const { glUniform1i(m_UniformKsSamplerLocation, unit); }
    void setUniformShininessSampler(GLuint unit) const { glUniform1i(m_UniformShininessSamplerLocation, unit); }
    void setUniformKaSamplerFactor(GLfloat factor) const { glUniform1f(m_UniformKaSamplerFactorLocation, factor); }
    void setUniformKdSamplerFactor(GLfloat factor) const { glUniform1f(m_UniformKdSamplerFactorLocation, factor); }
    void setUniformKsSamplerFactor(GLfloat factor) const { glUniform1f(m_UniformKsSamplerFactorLocation, factor); }
    void setUniformShininessSamplerFactor(GLfloat factor) const { glUniform1f(m_UniformShininessSamplerFactorLocation, factor); }
};


struct Camera: public glmlv::ViewController {
    float fovy = glm::radians(60.f), near = 0.01, far = 1000;
    size_t m_nWindowWidth = 0, m_nWindowHeight = 0;

    Camera() = delete;
    Camera(GLFWwindow* window, size_t w, size_t h, float speed = 1.f):
        glmlv::ViewController(window, speed),
        m_nWindowWidth(w),
        m_nWindowHeight(h) 
        {}

    float getAspect() const {
        assert(m_nWindowWidth);
        assert(m_nWindowHeight);
        return m_nWindowWidth / float(m_nWindowHeight);
    }
    glm::mat4 getProjMatrix() const {
        assert(fovy > 0);
        assert(fovy < glm::radians(180.f));
        return glm::perspective(fovy, getAspect(), near, far);
    }
};

struct MeshInstanceData {
    mat4 modelMatrix = mat4(1);
    vec3 color = vec3(1,1,1);
    GLuint textureUnit = 0;
};
struct SceneInstanceData {
    mat4 modelMatrix = mat4(1);
};

// A class that groups together a VAO, VBO and EBO.
// In essence, it is the on-GPU equivalent of SimpleGeometry.
// The render() method simply binds the VAO and calls glDrawElements().
class GLMesh {
    GLuint m_Vao = 0, m_Vbo = 0, m_Ibo = 0;
    GLsizei m_ElementCount = 0;
public:
    GLuint getVao() const { return m_Vao; }
    GLuint getVbo() const { return m_Vbo; }
    GLuint getIbo() const { return m_Ibo; }
    GLuint getElementCount() const { return m_ElementCount; }

    ~GLMesh() {
        glDeleteBuffers(1, &m_Ibo);
        glDeleteBuffers(1, &m_Vbo);
        glDeleteVertexArrays(1, &m_Vao);
    }
    GLMesh(const GLMesh& v) = delete;
    GLMesh& operator=(const GLMesh& v) = delete;
    GLMesh(GLMesh&& o): m_Vao(o.m_Vao), m_Vbo(o.m_Vbo), m_Ibo(o.m_Ibo), m_ElementCount(o.m_ElementCount) {
        o.m_Vao = o.m_Vbo = o.m_Ibo = 0;
    }
    GLMesh& operator=(GLMesh&& o) {
        std::swap(m_Vao, o.m_Vao);
        std::swap(m_Vbo, o.m_Vbo);
        std::swap(m_Ibo, o.m_Ibo);
        std::swap(m_ElementCount, o.m_ElementCount);
        return *this;
    }

    GLMesh() = delete;
    GLMesh(const SimpleGeometry& mesh) {
        m_ElementCount = mesh.indexBuffer.size();
        GLuint bufs[2] = {0, 0};
        glGenVertexArrays(1, &m_Vao);
        glGenBuffers(2, bufs);
        m_Vbo = bufs[0];
        m_Ibo = bufs[1];
        assert(m_Vao);
        assert(m_Vbo);
        assert(m_Ibo);
        const auto& v = mesh.vertexBuffer;
        const auto& i = mesh.indexBuffer;
        glBindVertexArray(m_Vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof v[0], v.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, i.size() * sizeof i[0], i.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        const auto stride = sizeof mesh.vertexBuffer[0];
        static_assert(std::is_same<const Vertex3f3f2f&, decltype(mesh.vertexBuffer[0])>::value, "Vertex types don't match!");
        static_assert(sizeof mesh.indexBuffer[0] == sizeof(uint32_t), "We are expecting to use GL_UNSIGNED_INT for indices!");
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex3f3f2f, position));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex3f3f2f, normal));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex3f3f2f, texCoords));
    }
    void render() const {
        glBindVertexArray(m_Vao);
        glDrawElements(GL_TRIANGLES, m_ElementCount, GL_UNSIGNED_INT, nullptr);
    }
    void render(const GLForwardRenderingProgram& prog, const Camera& camera, const MeshInstanceData& i) const {
        const auto& view = camera.getViewMatrix();
        const auto& proj = camera.getProjMatrix();
        mat4 modelView(view * i.modelMatrix);
        mat4 modelViewProj(proj * modelView);
        mat4 normalMatrix(transpose(inverse(modelView)));

        prog.setUniformKdSampler(i.textureUnit);
        prog.setUniformKd(i.color);
        prog.setUniformModelViewProjMatrix(modelViewProj);
        prog.setUniformModelViewMatrix(modelView);
        prog.setUniformNormalMatrix(normalMatrix);
        
        render(); // PERF: Binds VAO as many times as there are instances; useless.
    }
};

// A struct that groups a SimpleGeometry and GLMesh together; Essentially,
// both the on-CPU and on-GPU data for a mesh.
struct Mesh {
    SimpleGeometry m_SimpleGeometry;
    GLMesh m_GLMesh;
    Mesh() = delete;
    Mesh(const SimpleGeometry& mesh) : m_SimpleGeometry(mesh), m_GLMesh(mesh) {}
    void render() const {
        m_GLMesh.render();
    }
    void render(const GLForwardRenderingProgram& prog, const Camera& camera, const MeshInstanceData& i) const {
        m_GLMesh.render(prog, camera, i);
    }
};

// Pretty self-explanatory; Helper struct for initializing sampler parameters.
struct GLSamplerParams {
    GLenum m_WrapS = GL_REPEAT;
    GLenum m_WrapT = GL_REPEAT;
    GLenum m_MinFilter = GL_LINEAR;
    GLenum m_MagFilter = GL_LINEAR;
    GLSamplerParams withWrapST(GLenum v) const {
        auto p = *this;
        p.m_WrapS = p.m_WrapT = v;
        return p;
    }
    GLSamplerParams withMinMagFilter(GLenum v) const {
        auto p = *this;
        p.m_MinFilter = p.m_MagFilter = v;
        return p;
    }
};

// A RAII wrapper around an OpenGL sampler object.
class GLSampler {
    GLuint m_GLId = 0;
public:
    GLuint glId() const { return m_GLId; }

    ~GLSampler() {
        glDeleteSamplers(1, &m_GLId);
    }
    GLSampler(const GLSampler& v) = delete;
    GLSampler& operator=(const GLSampler& v) = delete;
    GLSampler(GLSampler&& o): m_GLId(o.m_GLId) { o.m_GLId = 0; }
    GLSampler& operator=(GLSampler&& o) { std::swap(m_GLId, o.m_GLId); return *this; }

    GLSampler() = delete;
    GLSampler(const GLSamplerParams& p) {
        glGenSamplers(1, &m_GLId);
        assert(m_GLId);
        glSamplerParameteri(m_GLId, GL_TEXTURE_WRAP_S, p.m_WrapS);
        glSamplerParameteri(m_GLId, GL_TEXTURE_WRAP_T, p.m_WrapT);
        glSamplerParameteri(m_GLId, GL_TEXTURE_MIN_FILTER, p.m_MinFilter);
        glSamplerParameteri(m_GLId, GL_TEXTURE_MAG_FILTER, p.m_MagFilter);
    }
    void bindToTextureUnit(GLuint unit) const {
        glBindSampler(unit, m_GLId);
    }
};

// A RAII wrapper around an OpenGL 2D texture object.
class GLTexture2D {
    GLuint m_GLId = 0;
public:
    GLuint glId() const { return m_GLId; }

    ~GLTexture2D() {
        glDeleteTextures(1, &m_GLId);
    }
    GLTexture2D(const GLTexture2D& v) = delete;
    GLTexture2D& operator=(const GLTexture2D& v) = delete;
    GLTexture2D(GLTexture2D&& o): m_GLId(o.m_GLId) { o.m_GLId = 0; }
    GLTexture2D& operator=(GLTexture2D&& o) { std::swap(m_GLId, o.m_GLId); return *this; }

    GLTexture2D(const glmlv::fs::path& path): GLTexture2D(glmlv::readImage(path)) {}
    GLTexture2D(const glmlv::Image2DRGBA& img) {
        glGenTextures(1, &m_GLId);
        assert(m_GLId);
        glBindTexture(GL_TEXTURE_2D, m_GLId);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, img.width(), img.height());
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 
            0, 0, img.width(), img.height(),
            GL_RGBA, GL_UNSIGNED_BYTE, img.data()
        );
    }
    void bind() const {
        glBindTexture(GL_TEXTURE_2D, m_GLId);
    }
};

// A scene loaded from an OBJ file; Contains both the on-CPU and on-GPU data.
struct Scene {
    ObjData m_ObjData;
    GLMesh m_GLMesh;
    std::vector<GLTexture2D> m_GLTextures2D;

    Scene() = delete;
    Scene(const glmlv::fs::path& path) :
        m_ObjData(objDataFromPath(path)),
        m_GLMesh(SimpleGeometry { m_ObjData.vertexBuffer, m_ObjData.indexBuffer })
    {
        for(const auto& img: m_ObjData.textures) {
            m_GLTextures2D.emplace_back(img);
        }
    }
    float getDiagonalLength() const {
        return glm::length(m_ObjData.bboxMax - m_ObjData.bboxMin);
    }

    void render(const GLForwardRenderingProgram& prog, const Camera& camera, const SceneInstanceData& instance) const {
        const auto& view = camera.getViewMatrix();
        const auto& proj = camera.getProjMatrix();
        mat4 modelView(view * instance.modelMatrix);
        mat4 modelViewProj(proj * modelView);
        mat4 normalMatrix(transpose(inverse(modelView)));

        prog.setUniformModelViewProjMatrix(modelViewProj);
        prog.setUniformModelViewMatrix(modelView);
        prog.setUniformNormalMatrix(normalMatrix);

        for(GLuint i=0 ; i<m_GLTextures2D.size() ; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            m_GLTextures2D[i].bind();
        }

        glBindVertexArray(m_GLMesh.getVao());

        auto indexOffset = 0u;
        for(auto i=0u ; i < m_ObjData.shapeCount ; ++i) {
            const auto matId = m_ObjData.materialIDPerShape[i];
            glmlv::ObjData::PhongMaterial mat; // Have a default one...
            if(matId >= 0) { // ...In case matId == -1
                mat = m_ObjData.materials[matId];
            }
            prog.setUniformKa(mat.Ka);
            prog.setUniformKd(mat.Kd);
            prog.setUniformKs(mat.Ks);
            prog.setUniformShininess(mat.shininess);
            prog.setUniformKaSampler(mat.KaTextureId);
            prog.setUniformKdSampler(mat.KdTextureId);
            prog.setUniformKsSampler(mat.KsTextureId);
            prog.setUniformShininessSampler(mat.shininessTextureId);
            prog.setUniformKaSamplerFactor(mat.KaTextureId >= 0);
            prog.setUniformKdSamplerFactor(mat.KdTextureId >= 0);
            prog.setUniformKsSamplerFactor(mat.KsTextureId >= 0);
            prog.setUniformShininessSamplerFactor(mat.shininessTextureId >= 0);

            const auto indexCount = m_ObjData.indexCountPerShape[i];
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*) (indexOffset * sizeof(GLuint)));
            indexOffset += indexCount;
        }
    }

private:
    static ObjData objDataFromPath(const glmlv::fs::path& path) {
        ObjData d;
        glmlv::loadObj(path, d);
        return d;
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
    const GLForwardRenderingProgram m_ForwardProgram;
    const GLSampler m_Sampler;
    const GLTexture2D m_CubeTex, m_SphereTex;
    const Mesh m_Cube, m_Sphere;
    const Scene m_Scene;
    Camera m_ViewController;
};
