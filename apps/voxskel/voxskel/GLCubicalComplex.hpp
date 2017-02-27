#pragma once

#include "glutils/GLutils.hpp"
#include "CubicalComplex3D.hpp"

namespace voxskel {

class GLCubicalComplex {
public:
    struct CCVertex{
        Vec3i m_position;

        CCVertex(Vec3i position):m_position(position) {}
    };

    const GLTexture3D& getTexture() const {
        return m_CCTexture;
    }

    void init(const CubicalComplex3D& cubicalComplex);

    void render() const {
        m_VAO.bind();
        glDrawArrays(GL_POINTS, 0, m_VBO.size());
        glBindVertexArray(0);
    }
private:
    GLBuffer<CCVertex> m_VBO;
    GLVertexArray m_VAO;
    GLTexture3D m_CCTexture;
};

}
