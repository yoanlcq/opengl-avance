#pragma once

#include <glm/glm.hpp>
#include "glutils/GLutils.hpp"

#include "GLVoxelFramebuffer.hpp"

namespace voxskel {

struct BBox3f
{
    glm::vec3 lower;
    glm::vec3 upper;
};

// Allow to voxelize a 3D Model using the method from Carlos Tripiana Montes
// Ref: http://voxskel.files.wordpress.com/2013/06/gpu-voxelization-c-tripiana-montes.pdf
class GLVoxelizerTripiana2009 {
public:
    static const GLuint VERTEX_ATTRIB_POSITION = 0u;

    GLVoxelizerTripiana2009(const std::string & shaderRootPath);

    ~GLVoxelizerTripiana2009();

    // Initialize required states of OpenGL
    // After calling this method, render the model with glDraw* functions.
    // The vertex attribute corresponding to the 3D positions must have
    // the index VERTEX_ATTRIB_POSITION in the VAO.
    //
    // \param bbox The bounding box of the model to voxelize
    // \param framebuffer The framebuffer in which to store the voxelization
    void initGLState(uint32_t resolution, BBox3f bbox,
                     GLVoxelFramebuffer& framebuffer, glm::mat4& gridToWorldMatrix);

    // Restore affected OpenGL states to their default values
    // Call this method after performing the rendering of the model
    void restoreGLState();

    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::vec3 tmporigBBox;
    float tmpvoxelSize;
    float m_AABCLength;
    int tmpnumVoxels;

private:
    GLProgram m_Program;

    VOXSKEL_GLUNIFORM(m_Program, glm::mat4, MVP);
    VOXSKEL_GLUNIFORM(m_Program, glm::vec2, halfPixelSize);
    VOXSKEL_GLUNIFORM(m_Program, int, numVoxels);
    VOXSKEL_GLUNIFORM(m_Program, glm::vec3, origBBox);
    VOXSKEL_GLUNIFORM(m_Program, int, numRenderTargets);
    VOXSKEL_GLUNIFORM(m_Program, float, voxelSize);
};

}
