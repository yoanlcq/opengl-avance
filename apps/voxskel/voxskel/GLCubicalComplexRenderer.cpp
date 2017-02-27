#include "GLCubicalComplexRenderer.hpp"

namespace voxskel {

void GLCubicalComplexRenderer::drawCubicalComplex(const glm::mat4& gridToWorld,
                                    const GLCubicalComplex& cubicalComplex) {
    glm::mat4 MVP = m_ProjectionMatrix * m_ViewMatrix * gridToWorld;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, cubicalComplex.getTexture().glId());

    m_RenderCubicalComplexFacesPass.m_Program.use();

    glUniformMatrix4fv(m_RenderCubicalComplexFacesPass.m_uMVPLocation, 1,
                       GL_FALSE, &MVP[0][0]);
    glUniform1i(m_RenderCubicalComplexFacesPass.m_uCubicalComplexLocation, 0);

    cubicalComplex.render();

    m_RenderCubicalComplexEdgesPass.m_Program.use();

    glUniformMatrix4fv(m_RenderCubicalComplexEdgesPass.m_uMVPLocation, 1,
                       GL_FALSE, &MVP[0][0]);
    glUniform1i(m_RenderCubicalComplexEdgesPass.m_uCubicalComplexLocation, 0);

    glLineWidth(4.f);

    cubicalComplex.render();

    m_RenderCubicalComplexPointsPass.m_Program.use();

    glUniformMatrix4fv(m_RenderCubicalComplexPointsPass.m_uMVPLocation, 1,
                       GL_FALSE, &MVP[0][0]);
    glUniform1i(m_RenderCubicalComplexPointsPass.m_uCubicalComplexLocation, 0);

    cubicalComplex.render();
}

GLCubicalComplexRenderer::GLCubicalComplexRenderer(const std::string & shaderRootPath):
    m_RenderCubicalComplexFacesPass(shaderRootPath),
    m_RenderCubicalComplexEdgesPass(shaderRootPath),
    m_RenderCubicalComplexPointsPass(shaderRootPath) {
}

GLCubicalComplexRenderer::RenderCubicalComplexFacesPass::RenderCubicalComplexFacesPass(const std::string & shaderRootPath):
    m_Program(buildProgram(shaderRootPath + "voxskel/CComplex.vs", shaderRootPath + "voxskel/CComplexFaces.gs", shaderRootPath + "voxskel/CComplex.fs")) {

    m_Program.use();

    m_uMVPLocation = m_Program.getUniformLocation("MVP");
    m_uMVLocation = m_Program.getUniformLocation("MV");
    m_uCubicalComplexLocation = m_Program.getUniformLocation("uCubicalComplex");
}

GLCubicalComplexRenderer::RenderCubicalComplexEdgesPass::RenderCubicalComplexEdgesPass(const std::string & shaderRootPath):
    m_Program(buildProgram("voxskel/CComplex.vs", shaderRootPath + "voxskel/CComplexEdges.gs", shaderRootPath + "voxskel/CComplex.fs")) {

    m_Program.use();

    m_uMVPLocation = m_Program.getUniformLocation("MVP");
    m_uMVLocation = m_Program.getUniformLocation("MV");
    m_uCubicalComplexLocation = m_Program.getUniformLocation("uCubicalComplex");
}

GLCubicalComplexRenderer::RenderCubicalComplexPointsPass::RenderCubicalComplexPointsPass(const std::string & shaderRootPath):
    m_Program(buildProgram(shaderRootPath + "voxskel/CComplex.vs", shaderRootPath + "voxskel/CComplexPoints.gs", shaderRootPath + "voxskel/CComplex.fs")) {

    m_Program.use();

    m_uMVPLocation = m_Program.getUniformLocation("MVP");
    m_uMVLocation = m_Program.getUniformLocation("MV");
    m_uCubicalComplexLocation = m_Program.getUniformLocation("uCubicalComplex");
}

}
