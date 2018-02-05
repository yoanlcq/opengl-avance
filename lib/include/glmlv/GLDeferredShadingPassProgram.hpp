#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "filesystem.hpp"
#include "load_obj.hpp"
#include "GLProgram.hpp"
#include "ViewController.hpp"

namespace glmlv
{

class GLDeferredShadingPassProgram: public GLProgram {
    const GLint m_UniformGPositionLocation                    = -1;
    const GLint m_UniformGNormalLocation                      = -1;
    const GLint m_UniformGAmbientLocation                     = -1;
    const GLint m_UniformGDiffuseLocation                     = -1;
    const GLint m_UniformGGlossyShininessLocation             = -1;
    const GLint m_UniformDirectionalLightDirLocation          = -1;
    const GLint m_UniformDirectionalLightIntensityLocation    = -1;
    const GLint m_UniformPointLightPositionLocation           = -1;
    const GLint m_UniformPointLightIntensityLocation          = -1;
    const GLint m_UniformPointLightRangeLocation              = -1;
    const GLint m_UniformPointLightAttenuationFactorLocation  = -1;
    const GLint m_UniformPointLightCountLocation              = -1;
    const GLint m_UniformDirLightViewProjMatrix               = -1;
    const GLint m_UniformDirLightShadowMap                    = -1;
    const GLint m_UniformDirLightShadowMapBias                = -1;
    const GLint m_UniformDirLightShadowMapSampleCountLocation = -1;
    const GLint m_UniformDirLightShadowMapSpreadLocation      = -1;


public:
    GLDeferredShadingPassProgram(const fs::path& vs, const fs::path& fs):
        GLProgram(compileProgram({ vs.string(), fs.string() })),
        m_UniformGPositionLocation                    (getUniformLocation("uGPosition")),
        m_UniformGNormalLocation                      (getUniformLocation("uGNormal")),
        m_UniformGAmbientLocation                     (getUniformLocation("uGAmbient")),
        m_UniformGDiffuseLocation                     (getUniformLocation("uGDiffuse")),
        m_UniformGGlossyShininessLocation             (getUniformLocation("uGGlossyShininess")),
        m_UniformDirectionalLightDirLocation          (getUniformLocation("uDirectionalLightDir")),
        m_UniformDirectionalLightIntensityLocation    (getUniformLocation("uDirectionalLightIntensity")),
        m_UniformPointLightPositionLocation           (getUniformLocation("uPointLightPosition")),
        m_UniformPointLightIntensityLocation          (getUniformLocation("uPointLightIntensity")),
        m_UniformPointLightRangeLocation              (getUniformLocation("uPointLightRange")),
        m_UniformPointLightAttenuationFactorLocation  (getUniformLocation("uPointLightAttenuationFactor")),
        m_UniformPointLightCountLocation              (getUniformLocation("uPointLightCount")),
        m_UniformDirLightViewProjMatrix               (getUniformLocation("uDirLightViewProjMatrix")),
        m_UniformDirLightShadowMap                    (getUniformLocation("uDirLightShadowMap")),
        m_UniformDirLightShadowMapBias                (getUniformLocation("uDirLightShadowMapBias")),
        m_UniformDirLightShadowMapSampleCountLocation (getUniformLocation("uDirLightShadowMapSampleCount")),
        m_UniformDirLightShadowMapSpreadLocation      (getUniformLocation("uDirLightShadowMapSpread"))
        {}

    // NOTE: Keep in sync with forward shader
    static const size_t MAX_POINT_LIGHTS = 32;

    // XXX: I'm not actually of fan of forcing implicit world-space to view-space mapping
    // when uploading uniforms
    struct LightingUniforms {
        glm::vec3 dirLightDir = glm::vec3(1,0,0);
        glm::vec3 dirLightIntensity = glm::vec3(1,1,1);
        GLfloat dirLightShadowMapBias = 0;
        GLint dirLightShadowMap = 0;
        GLint dirLightShadowMapSampleCount = 1;
        GLfloat dirLightShadowMapSpread = 0.0005;
        glm::mat4 dirLightViewProjMatrix = glm::mat4(1);
        size_t pointLightCount = 1;
        glm::vec3 pointLightPosition[MAX_POINT_LIGHTS] = {};
        glm::vec3 pointLightIntensity[MAX_POINT_LIGHTS] = {};
        GLfloat pointLightRange[MAX_POINT_LIGHTS] = {};
        GLfloat pointLightAttenuationFactor[MAX_POINT_LIGHTS] = {};
    };

    void setLightingUniforms(const LightingUniforms& d, const ViewController& vc) const {
        auto count = d.pointLightCount;
        assert(count < MAX_POINT_LIGHTS);
        setUniformDirectionalLightDir(d.dirLightDir, vc);
        setUniformDirectionalLightIntensity(d.dirLightIntensity);
        setUniformDirLightViewProjMatrix(d.dirLightViewProjMatrix);
        setUniformDirLightShadowMap(d.dirLightShadowMap);
        setUniformDirLightShadowMapBias(d.dirLightShadowMapBias);
        setUniformDirLightShadowMapSampleCount(d.dirLightShadowMapSampleCount);
        setUniformDirLightShadowMapSpread(d.dirLightShadowMapSpread);
        setUniformPointLightCount(count);
        setUniformPointLightPosition(count, d.pointLightPosition, vc);
        setUniformPointLightIntensity(count, d.pointLightIntensity);
        setUniformPointLightRange(count, d.pointLightRange);
        setUniformPointLightAttenuationFactor(count, d.pointLightAttenuationFactor);
    }

    void setUniformDirectionalLightDir(const glm::vec3& v, const ViewController& vc) const {
        auto view = vc.getViewMatrix();
        auto data = glm::vec3(view * glm::vec4(v, 0));
        glUniform3fv(m_UniformDirectionalLightDirLocation, 1, &data[0]);
    }
    void setUniformDirectionalLightIntensity(const glm::vec3& v) const {
        glUniform3fv(m_UniformDirectionalLightIntensityLocation, 1, &v[0]);
    }

    void setUniformDirLightViewProjMatrix(const glm::mat4 &m) const {
        glUniformMatrix4fv(m_UniformDirLightViewProjMatrix, 1, GL_FALSE, &m[0][0]);
    }
    void setUniformDirLightShadowMap(GLint i) const {
        glUniform1i(m_UniformDirLightShadowMap, i);
    }
    void setUniformDirLightShadowMapBias(GLfloat bias) const {
        glUniform1f(m_UniformDirLightShadowMapBias, bias);
    }
    void setUniformDirLightShadowMapSampleCount(GLint i) const {
        glUniform1i(m_UniformDirLightShadowMapSampleCountLocation, i);
    }
    void setUniformDirLightShadowMapSpread(GLfloat v) const {
        glUniform1f(m_UniformDirLightShadowMapSpreadLocation, v);
    }


    void setUniformPointLightPosition(size_t count, const glm::vec3* v, const ViewController& vc) const {
        assert(count < MAX_POINT_LIGHTS);
        auto view = vc.getViewMatrix();
        glm::vec3 vsPointLightPosition[MAX_POINT_LIGHTS] = {};
        for(size_t i=0 ; i<MAX_POINT_LIGHTS ; ++i) {
            vsPointLightPosition[i] = glm::vec3(view * glm::vec4(v[i], 1));
        }
        glUniform3fv(m_UniformPointLightPositionLocation, count, &vsPointLightPosition[0][0]);
    }
    void setUniformPointLightIntensity(size_t count, const glm::vec3* v) const {
        assert(count < MAX_POINT_LIGHTS);
        glUniform3fv(m_UniformPointLightIntensityLocation, count, &v[0][0]);
    }
    void setUniformPointLightRange(size_t count, const GLfloat* factor) const {
        assert(count < MAX_POINT_LIGHTS);
        glUniform1fv(m_UniformPointLightRangeLocation, count, factor);
    }
    void setUniformPointLightAttenuationFactor(size_t count, const GLfloat* factor) const {
        assert(count < MAX_POINT_LIGHTS);
        glUniform1fv(m_UniformPointLightAttenuationFactorLocation, count, factor);
    }
    void setUniformPointLightCount(size_t count) const {
        assert(count < MAX_POINT_LIGHTS);
        glUniform1ui(m_UniformPointLightCountLocation, count);
    }
    void setUniformGPosition       (GLint i) const { glUniform1i(m_UniformGPositionLocation, i); }
    void setUniformGNormal         (GLint i) const { glUniform1i(m_UniformGNormalLocation, i); }
    void setUniformGAmbient        (GLint i) const { glUniform1i(m_UniformGAmbientLocation, i); }
    void setUniformGDiffuse        (GLint i) const { glUniform1i(m_UniformGDiffuseLocation, i); }
    void setUniformGGlossyShininess(GLint i) const { glUniform1i(m_UniformGGlossyShininessLocation, i); }
};

} // namespace glmlv
