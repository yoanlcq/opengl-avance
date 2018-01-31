#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "filesystem.hpp"
#include "load_obj.hpp"
#include "GLProgram.hpp"
#include "ViewController.hpp"

namespace glmlv
{

class GLForwardRenderingProgram: public GLProgram {
    const GLint m_UniformModelViewProjMatrixLocation         = -1;
    const GLint m_UniformModelViewMatrixLocation             = -1;
    const GLint m_UniformNormalMatrixLocation                = -1;
    const GLint m_UniformDirectionalLightDirLocation         = -1;
    const GLint m_UniformDirectionalLightIntensityLocation   = -1;
    const GLint m_UniformPointLightPositionLocation          = -1;
    const GLint m_UniformPointLightIntensityLocation         = -1;
    const GLint m_UniformPointLightRangeLocation             = -1;
    const GLint m_UniformPointLightAttenuationFactorLocation = -1;
    const GLint m_UniformPointLightCountLocation             = -1;
    const GLint m_UniformKaLocation                          = -1;
    const GLint m_UniformKdLocation                          = -1;
    const GLint m_UniformKsLocation                          = -1;
    const GLint m_UniformShininessLocation                   = -1;
    const GLint m_UniformKaSamplerLocation                   = -1;
    const GLint m_UniformKdSamplerLocation                   = -1;
    const GLint m_UniformKsSamplerLocation                   = -1;
    const GLint m_UniformShininessSamplerLocation            = -1;
    const GLint m_UniformKaSamplerFactorLocation             = -1;
    const GLint m_UniformKdSamplerFactorLocation             = -1;
    const GLint m_UniformKsSamplerFactorLocation             = -1;
    const GLint m_UniformShininessSamplerFactorLocation      = -1;

public:
    GLForwardRenderingProgram(const fs::path& vs, const fs::path& fs):
        GLProgram(compileProgram({ vs.string(), fs.string() })),
        m_UniformModelViewProjMatrixLocation         (getUniformLocation("uModelViewProjMatrix")),
        m_UniformModelViewMatrixLocation             (getUniformLocation("uModelViewMatrix")),
        m_UniformNormalMatrixLocation                (getUniformLocation("uNormalMatrix")),
        m_UniformDirectionalLightDirLocation         (getUniformLocation("uDirectionalLightDir")),
        m_UniformDirectionalLightIntensityLocation   (getUniformLocation("uDirectionalLightIntensity")),
        m_UniformPointLightPositionLocation          (getUniformLocation("uPointLightPosition")),
        m_UniformPointLightIntensityLocation         (getUniformLocation("uPointLightIntensity")),
        m_UniformPointLightRangeLocation             (getUniformLocation("uPointLightRange")),
        m_UniformPointLightAttenuationFactorLocation (getUniformLocation("uPointLightAttenuationFactor")),
        m_UniformPointLightCountLocation             (getUniformLocation("uPointLightCount")),
        m_UniformKaLocation                          (getUniformLocation("uKa")),
        m_UniformKdLocation                          (getUniformLocation("uKd")),
        m_UniformKsLocation                          (getUniformLocation("uKs")),
        m_UniformShininessLocation                   (getUniformLocation("uShininess")),
        m_UniformKaSamplerLocation                   (getUniformLocation("uKaSampler")),
        m_UniformKdSamplerLocation                   (getUniformLocation("uKdSampler")),
        m_UniformKsSamplerLocation                   (getUniformLocation("uKsSampler")),
        m_UniformShininessSamplerLocation            (getUniformLocation("uShininessSampler")),
        m_UniformKaSamplerFactorLocation             (getUniformLocation("uKaSamplerFactor")),
        m_UniformKdSamplerFactorLocation             (getUniformLocation("uKdSamplerFactor")),
        m_UniformKsSamplerFactorLocation             (getUniformLocation("uKsSamplerFactor")),
        m_UniformShininessSamplerFactorLocation      (getUniformLocation("uShininessSamplerFactor"))
        {}

    static const GLuint MAX_POINT_LIGHTS = 32;

    struct LightingUniforms {
        glm::vec3 dirLightDir = glm::vec3(1,0,0);
        glm::vec3 dirLightIntensity = glm::vec3(1,1,1);
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
        setUniformPointLightCount(count);
        setUniformPointLightPosition(count, d.pointLightPosition, vc);
        setUniformPointLightIntensity(count, d.pointLightIntensity);
        setUniformPointLightRange(count, d.pointLightRange);
        setUniformPointLightAttenuationFactor(count, d.pointLightAttenuationFactor);
    }

    void setUniformModelViewProjMatrix(const glm::mat4& m) const {
        glUniformMatrix4fv(m_UniformModelViewProjMatrixLocation, 1, GL_FALSE, &m[0][0]);
    }
    void setUniformModelViewMatrix(const glm::mat4& m) const {
        glUniformMatrix4fv(m_UniformModelViewMatrixLocation, 1, GL_FALSE, &m[0][0]);
    }
    void setUniformNormalMatrix(const glm::mat4& m) const {
        glUniformMatrix4fv(m_UniformNormalMatrixLocation, 1, GL_FALSE, &m[0][0]);
    }
    void setUniformDirectionalLightDir(const glm::vec3& v, const ViewController& vc) const {
        auto view = vc.getViewMatrix();
        auto data = glm::vec3(view * glm::vec4(v, 0));
        glUniform3fv(m_UniformDirectionalLightDirLocation, 1, &data[0]);
    }
    void setUniformDirectionalLightIntensity(const glm::vec3& v) const {
        glUniform3fv(m_UniformDirectionalLightIntensityLocation, 1, &v[0]);
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
    void setUniformKa(const glm::vec3& v) const { glUniform3fv(m_UniformKaLocation, 1, &v[0]); }
    void setUniformKd(const glm::vec3& v) const { glUniform3fv(m_UniformKdLocation, 1, &v[0]); }
    void setUniformKs(const glm::vec3& v) const { glUniform3fv(m_UniformKsLocation, 1, &v[0]); }
    void setUniformShininess(GLfloat v) const { glUniform1f(m_UniformShininessLocation, v); }
    void setUniformKaSampler(GLuint unit) const { glUniform1i(m_UniformKaSamplerLocation, unit); }
    void setUniformKdSampler(GLuint unit) const { glUniform1i(m_UniformKdSamplerLocation, unit); }
    void setUniformKsSampler(GLuint unit) const { glUniform1i(m_UniformKsSamplerLocation, unit); }
    void setUniformShininessSampler(GLuint unit) const { glUniform1i(m_UniformShininessSamplerLocation, unit); }
    void setUniformKaSamplerFactor(GLfloat factor) const { glUniform1f(m_UniformKaSamplerFactorLocation, factor); }
    void setUniformKdSamplerFactor(GLfloat factor) const { glUniform1f(m_UniformKdSamplerFactorLocation, factor); }
    void setUniformKsSamplerFactor(GLfloat factor) const { glUniform1f(m_UniformKsSamplerFactorLocation, factor); }
    void setUniformShininessSamplerFactor(GLfloat factor) const { glUniform1f(m_UniformShininessSamplerFactorLocation, factor); }

    void resetMaterialUniforms() const {
        const ObjData::PhongMaterial mat; // Use the default one
        setUniformKa(mat.Ka);
        setUniformKd(mat.Kd);
        setUniformKs(mat.Ks);
        setUniformShininess(1); // NOTE: 1 instead of mat.shininess (i.e 0) fixes completely black fragments on NVIDIA
        setUniformKaSampler(mat.KaTextureId >= 0 ? mat.KaTextureId : 0);
        setUniformKdSampler(mat.KdTextureId >= 0 ? mat.KdTextureId : 0);
        setUniformKsSampler(mat.KsTextureId >= 0 ? mat.KsTextureId : 0);
        setUniformShininessSampler(mat.shininessTextureId >= 0 ? mat.shininessTextureId : 0);
        setUniformKaSamplerFactor(mat.KaTextureId >= 0);
        setUniformKdSamplerFactor(mat.KdTextureId >= 0);
        setUniformKsSamplerFactor(mat.KsTextureId >= 0);
        setUniformShininessSamplerFactor(mat.shininessTextureId >= 0);
    }
};

} // namespace glmlv
