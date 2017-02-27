#pragma once

#include <memory>
#include "glutils/opengl.hpp"
#include "CubicalComplex3D.hpp"

namespace voxskel {

// An OpenGL framebuffer to store a voxelization
class GLVoxelFramebuffer {
private:
    GLuint m_FBO = 0u;
    std::unique_ptr<GLuint[]> m_renderedTexture;
    int m_width;
    int m_height;
    int m_depth;
    int m_numTextures;

    // Can't be copied
    GLVoxelFramebuffer(const GLVoxelFramebuffer&) = delete;
    const GLVoxelFramebuffer& operator =(const GLVoxelFramebuffer&) = delete;

public:
    GLVoxelFramebuffer();

    ~GLVoxelFramebuffer();

    bool init(int width, int height, int depth, int m_numTextures);

    void bind(GLenum e) const{
        glBindFramebuffer(e, m_FBO);
    }

    GLuint getRenderedTexture(int i) const {
        return m_renderedTexture[i];
    }

    int width() const {
        return m_width;
    }

    int height() const {
        return m_height;
    }

    int depth() const {
        return m_depth;
    }

    int getTextureCount() const {
        return m_numTextures;
    }
};

VoxelGrid getVoxelGrid(const GLVoxelFramebuffer& framebuffer, bool getComplementary = false);

CubicalComplex3D getCubicalComplexFromVoxelFramebuffer(const GLVoxelFramebuffer& framebuffer, bool getComplementary = false);

}
