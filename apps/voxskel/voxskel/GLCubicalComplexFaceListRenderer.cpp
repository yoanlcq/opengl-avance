#include "GLCubicalComplexFaceListRenderer.hpp"

namespace voxskel {

void GLCubicalComplexFaceList::init(const Vertex* vertices, size_t count) {
    m_VBO.setData(count, vertices, GL_STATIC_DRAW);

    const auto bindingIdx = 0;
    glVertexArrayVertexBuffer(m_VAO.glId(), bindingIdx, m_VBO.glId(), 0, sizeof(Vertex));

    m_VAO.enableVertexAttrib(FACE_ATTR_LOCATION);
    m_VAO.enableVertexAttrib(FACECOLORXY_ATTR_LOCATION);
    m_VAO.enableVertexAttrib(FACECOLORYZ_LOCATION);
    m_VAO.enableVertexAttrib(FACECOLORXZ_ATTR_LOCATION);

    glVertexArrayAttribBinding(m_VAO.glId(), FACE_ATTR_LOCATION, bindingIdx);
    glVertexArrayAttribBinding(m_VAO.glId(), FACECOLORXY_ATTR_LOCATION, bindingIdx);
    glVertexArrayAttribBinding(m_VAO.glId(), FACECOLORYZ_LOCATION, bindingIdx);
    glVertexArrayAttribBinding(m_VAO.glId(), FACECOLORXZ_ATTR_LOCATION, bindingIdx);

    glVertexArrayAttribIFormat(m_VAO.glId(), FACE_ATTR_LOCATION, 4, GL_INT, VOXSKEL_OFFSETOF(Vertex, m_Face));
    glVertexArrayAttribFormat(m_VAO.glId(), FACECOLORXY_ATTR_LOCATION, 3, GL_FLOAT, GL_FALSE, VOXSKEL_OFFSETOF(Vertex, m_FaceColorXY));
    glVertexArrayAttribFormat(m_VAO.glId(), FACECOLORYZ_LOCATION, 3, GL_FLOAT, GL_FALSE, VOXSKEL_OFFSETOF(Vertex, m_FaceColorYZ));
    glVertexArrayAttribFormat(m_VAO.glId(), FACECOLORXZ_ATTR_LOCATION, 3, GL_FLOAT, GL_FALSE, VOXSKEL_OFFSETOF(Vertex, m_FaceColorXZ));
}

void GLCubicalComplexFaceList::render() const {
    m_VAO.bind();
    glDrawArrays(GL_POINTS, 0, m_VBO.size());
}

GLCubicalComplexFaceListRenderer::GLCubicalComplexFaceListRenderer(
        const std::string & shaderRootPath):
    m_Program(buildProgram(shaderRootPath + "voxskel/CComplexFaceList.vs",
        shaderRootPath + "voxskel/CComplexFaceList.gs",
        shaderRootPath + "voxskel/CComplexFaceList.fs")) {
}

void GLCubicalComplexFaceListRenderer::render(
        const GLCubicalComplexFaceList& faceList) {
    glEnable(GL_DEPTH_TEST);

    m_Program.use();

    uMVPMatrix.set(m_MVPMatrix);

    faceList.render();

    glUseProgram(0u);
    glDisable(GL_DEPTH_TEST);
}

}
