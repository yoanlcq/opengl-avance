#include "GLCubicalComplex.hpp"
#include <iostream>

namespace voxskel {

void GLCubicalComplex::init(const CubicalComplex3D &cubicalComplex) {
    int size_x = cubicalComplex.width();
    int size_y = cubicalComplex.height();
    int size_z = cubicalComplex.depth();

    std::vector<int32_t> tmp(size_x * size_y * size_z);

    for(auto i = 0u; i < tmp.size(); ++i) {
        tmp[i] = cubicalComplex[i];
    }

    m_CCTexture.setMagFilter(GL_NEAREST);
    m_CCTexture.setMinFilter(GL_NEAREST);
    m_CCTexture.setStorage(1, GL_R8I, size_x, size_y, size_z);
    m_CCTexture.setSubImage(0, size_x, size_y, size_z,
                         GL_RED_INTEGER, GL_INT, tmp.data());

    std::vector<CCVertex> CCVertices;

    for(int z = 0; z < size_z; ++z) {
        for(int y = 0; y < size_y; ++y) {
            for(int x = 0; x < size_x; ++x) {
                Vec3i position{ x, y, z };
                CCVertices.emplace_back(CCVertex(position));
            }
        }
    }

    m_VBO.setData(CCVertices, GL_STATIC_DRAW);
    const auto attrLocation = 0;
    m_VAO.enableVertexAttrib(attrLocation);
    const auto bindingIdx = 0;
    glVertexArrayVertexBuffer(m_VAO.glId(), bindingIdx, m_VBO.glId(), 0, sizeof(CCVertex));
    glVertexArrayAttribBinding(m_VAO.glId(), attrLocation, bindingIdx);
    glVertexArrayAttribIFormat(m_VAO.glId(), attrLocation, 3, GL_INT, 0);
}

}
