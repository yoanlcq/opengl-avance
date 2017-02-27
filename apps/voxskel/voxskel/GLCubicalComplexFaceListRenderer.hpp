#pragma once

#include <glm/glm.hpp>
#include "glutils/GLutils.hpp"
#include "types.hpp"

#include "CubicalComplex3D.hpp"

namespace voxskel {

class GLCubicalComplexFaceList {
public:
    static const GLuint FACE_ATTR_LOCATION = 0u;
    static const GLuint FACECOLORXY_ATTR_LOCATION = 1u;
    static const GLuint FACECOLORYZ_LOCATION = 2u;
    static const GLuint FACECOLORXZ_ATTR_LOCATION = 3u;

    struct Vertex {
        Vec4i m_Face; // x,y,z are the coordinates of the voxel, w is the bitfield
        Vec3f m_FaceColorXY;
        Vec3f m_FaceColorYZ;
        Vec3f m_FaceColorXZ;

        Vertex(const Vec4i& face,
               const Vec3f& faceColorXY,
               const Vec3f& faceColorYZ,
               const Vec3f& faceColorXZ):
            m_Face(face),
            m_FaceColorXY(faceColorXY),
            m_FaceColorYZ(faceColorYZ),
            m_FaceColorXZ(faceColorXZ) {
        }
    };

    void init(const Vertex* vertices, size_t count);

    void render() const;

private:
    GLBuffer<Vertex> m_VBO;
    GLVertexArray m_VAO;
};

class GLCubicalComplexFaceListRenderer {
public:
    GLCubicalComplexFaceListRenderer(const std::string & shaderRootPath);

    void setMVMPatrix(const glm::mat4& mvpMatrix) {
        m_MVPMatrix = mvpMatrix;
    }

    void render(const GLCubicalComplexFaceList& faceList);

private:
    glm::mat4 m_MVPMatrix;

    GLProgram m_Program;
    VOXSKEL_GLUNIFORM(m_Program, glm::mat4, uMVPMatrix);
};

}
