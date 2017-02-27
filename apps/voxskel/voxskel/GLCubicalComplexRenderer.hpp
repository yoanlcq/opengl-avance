#pragma once

#include <glm/glm.hpp>
#include "GLCubicalComplex.hpp"

namespace voxskel {

class GLCubicalComplexRenderer {
public:
    GLCubicalComplexRenderer(const std::string & shaderRootPath);

    void setProjectionMatrix(const glm::mat4 & projectionMatrix) {
        m_ProjectionMatrix = projectionMatrix;
    }

    void setViewMatrix(const glm::mat4 & viewMatrix) {
        m_ViewMatrix = viewMatrix;
    }

    void drawCubicalComplex(const glm::mat4& gridToWorld,
                            const GLCubicalComplex& cubicalComplex);

private:
    glm::mat4 m_ProjectionMatrix;
    glm::mat4 m_ViewMatrix;

    struct RenderCubicalComplexFacesPass {
        GLProgram m_Program;

        GLint m_uMVLocation;
        GLint m_uMVPLocation;
        GLint m_uCubicalComplexLocation;

        RenderCubicalComplexFacesPass(const std::string & shaderRootPath);
    };
    RenderCubicalComplexFacesPass m_RenderCubicalComplexFacesPass;

    struct RenderCubicalComplexEdgesPass {
        GLProgram m_Program;

        GLint m_uMVLocation;
        GLint m_uMVPLocation;
        GLint m_uCubicalComplexLocation;

        RenderCubicalComplexEdgesPass(const std::string & shaderRootPath);
    };
    RenderCubicalComplexEdgesPass m_RenderCubicalComplexEdgesPass;

    struct RenderCubicalComplexPointsPass {
        GLProgram m_Program;

        GLint m_uMVLocation;
        GLint m_uMVPLocation;
        GLint m_uCubicalComplexLocation;

        RenderCubicalComplexPointsPass(const std::string & shaderRootPath);
    };
    RenderCubicalComplexPointsPass m_RenderCubicalComplexPointsPass;
};

}
