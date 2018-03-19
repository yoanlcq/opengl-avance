#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLDeferredGPassProgram.hpp>
#include <glmlv/GLDeferredShadingPassProgram.hpp>
#include <glmlv/GLDirectionalSMProgram.hpp>
#include <glmlv/GLDisplayDepthMapProgram.hpp>
#include <glmlv/GLMaterialProgram.hpp>
#include <glmlv/GLTexture2D.hpp>
#include <glmlv/GLCubeMapTexture.hpp>
#include <glmlv/GLSampler.hpp>
#include <glmlv/Scene.hpp>
#include <glmlv/Camera.hpp>
#include <glmlv/GlobalWavPlayer.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <functional>
#include <map>
#include <array>

void handleFboStatus(GLenum status);

glmlv::Image2DRGBA readImageNoException(const glmlv::fs::path& path) noexcept;

class GLScreenQuadProgram: public glmlv::GLProgram {
    GLint m_UniformScaleLocation            = -1;
    GLint m_UniformPositionLocation         = -1;
    GLint m_UniformTextureLocation          = -1;
    GLint m_UniformTexCoordsOffsetLocation  = -1;
    GLint m_UniformTexCoordsSizeLocation    = -1;
    GLint m_UniformAlphaMultiplierLocation  = -1;

public:
    GLScreenQuadProgram(const glmlv::fs::path& vs, const glmlv::fs::path& fs):
        GLProgram(glmlv::compileProgram({ vs.string(), fs.string() })),
        m_UniformScaleLocation           (getUniformLocation("uScale")),
        m_UniformPositionLocation        (getUniformLocation("uPosition")),
        m_UniformTextureLocation         (getUniformLocation("uTexture")),
        m_UniformTexCoordsOffsetLocation (getUniformLocation("uTexCoordsOffset")),
        m_UniformTexCoordsSizeLocation   (getUniformLocation("uTexCoordsSize")),
        m_UniformAlphaMultiplierLocation (getUniformLocation("uAlphaMultiplier"))
        {}
    void setUniformScale(const glm::vec2& v) const { glUniform2fv(m_UniformScaleLocation, 1, &v[0]); }
    void setUniformPosition(const glm::vec2& v) const { glUniform2fv(m_UniformPositionLocation, 1, &v[0]); }
    void setUniformTexture(GLint i) const { glUniform1i(m_UniformTextureLocation, i); }
    void setUniformTexCoordsOffset(const glm::vec2& v) const { glUniform2fv(m_UniformTexCoordsOffsetLocation, 1, &v[0]); }
    void setUniformTexCoordsSize(const glm::vec2& v) const { glUniform2fv(m_UniformTexCoordsSizeLocation, 1, &v[0]); }
    void setUniformAlphaMultipler(float f) const { glUniform1f(m_UniformAlphaMultiplierLocation, f); }
};

class GLParticlesSimulationProgram: public glmlv::GLProgram {
    GLint m_UniformParticleCountLocation = -1;
    GLint m_UniformDeltaTimeLocation     = -1;
    GLint m_UniformVelMultiplierLocation = -1;

public:
    GLParticlesSimulationProgram(const glmlv::fs::path& cs):
        GLProgram(glmlv::compileProgram({ cs.string() })),
        m_UniformParticleCountLocation (getUniformLocation("uParticleCount")),
        m_UniformDeltaTimeLocation     (getUniformLocation("uDeltaTime")),
        m_UniformVelMultiplierLocation (getUniformLocation("uVelMultiplier"))
        {}
    void setUniformParticleCount(size_t count) const { glUniform1ui(m_UniformParticleCountLocation, count); }
    void setUniformDeltaTime(float dt) const { glUniform1f(m_UniformDeltaTimeLocation, dt); }
    void setUniformVelMultiplier(float f) const { glUniform1f(m_UniformVelMultiplierLocation, f); }
};


class GLParticlesRenderingProgram: public glmlv::GLProgram {
    const GLint m_UniformModelViewProjMatrixLocation = -1;
    const GLint m_UniformPointSizeLocation           = -1;
    const GLint m_UniformZScaleLocation              = -1;
    const GLint m_UniformZInfluenceLocation          = -1;
    const GLint m_UniformInnerColorLocation          = -1;
    const GLint m_UniformOuterColorLocation          = -1;
public:
    GLParticlesRenderingProgram(const glmlv::fs::path& vs, const glmlv::fs::path& fs):
        GLProgram(glmlv::compileProgram({ vs.string(), fs.string() })),
        m_UniformModelViewProjMatrixLocation(getUniformLocation("uModelViewProjMatrix")),
        m_UniformPointSizeLocation          (getUniformLocation("uPointSize")),
        m_UniformZScaleLocation             (getUniformLocation("uZScale")),
        m_UniformZInfluenceLocation         (getUniformLocation("uZInfluence")),
        m_UniformInnerColorLocation         (getUniformLocation("uInnerColor")),
        m_UniformOuterColorLocation         (getUniformLocation("uOuterColor"))
        {}
    void setUniformModelViewProjMatrix(const glm::mat4& m) const { glUniformMatrix4fv(m_UniformModelViewProjMatrixLocation, 1, GL_FALSE, &m[0][0]); }
    void setUniformPointSize(float f) const { glUniform1f(m_UniformPointSizeLocation, f); }
    void setUniformZScale(float f) const { glUniform1f(m_UniformZScaleLocation, f); }
    void setUniformZInfluence(float f) const { glUniform1f(m_UniformZInfluenceLocation, f); }
    void setUniformInnerColor(const glm::vec4 &v) const { glUniform4fv(m_UniformInnerColorLocation, 1, &v[0]); }
    void setUniformOuterColor(const glm::vec4 &v) const { glUniform4fv(m_UniformOuterColorLocation, 1, &v[0]); }
};

class Particles {
public:
    enum class Shape {
        Sphere,
        Disk
    };
    struct InstanceData {
        // Compute shader - update
        float velMultiplier   = 1.f;
        // Vertex-Fragment shader - render
        glm::vec3 origin      = glm::vec3(0.f);
        glm::vec3 forward     = glm::vec3(1.f);
        float pointSize       = 8.f;
        float zScale          = 1000.f;
        float zInfluence      = 1.f;
        glm::vec4 innerColor  = glm::vec4(1.f, 0.9f, 0.3f, 1.0f);
        glm::vec4 outerColor  = glm::vec4(1.f, 0.2f, 0.0f, 0.0f);

        glm::mat4 getTranslationMatrix() const {
            return glm::translate(glm::mat4(1.f), origin);
        }
        glm::mat4 getRotationMatrix() const {
            return glm::mat4_cast(glmlv::SceneInstanceData::quatLookAt(forward, glm::vec3(0,1,0)));
        }
        glm::mat4 getModelMatrix() const {
            return getTranslationMatrix() * getRotationMatrix();
        }
    };

private:

    GLuint m_Vao = 0, m_PositionBo = 0, m_VelocityBo = 0;
    const size_t m_MaxParticleCount;
    size_t m_ParticleCount;

    struct HostData {
        std::vector<glm::vec4> pos, vel;

        HostData(size_t count, Shape shape, float radius, const InstanceData& instanceData):
            pos(count), vel(count)
        {
            const auto m = instanceData.getModelMatrix();
            switch(shape) {
            case Shape::Sphere:
                for(size_t i=0 ; i<count ; ++i) {
                    pos[i] = m * glm::vec4(glm::sphericalRand(radius), 1);
                    vel[i] = m * glm::vec4(glm::sphericalRand(radius) / 10.f, 0);
                }
                break;
            case Shape::Disk:
                for(size_t i=0 ; i<count ; ++i) {
                    const auto ax = glm::radians(glm::linearRand(-20.f, 20.f));
                    const auto ay = glm::radians(glm::linearRand(-20.f, 20.f));
                    const auto rx = glm::rotate(glm::mat4(1), ax, glm::vec3(1,0,0));
                    const auto ry = glm::rotate(glm::mat4(1), ay, glm::vec3(0,1,0));
                    vel[i] = m * rx * ry * glm::vec4(0,0,-1,0);
                    pos[i] = m * glm::vec4(glm::diskRand(radius), 0, 1);
                }
                break;
            }
        }
    };

public:

    Shape m_Shape;
    float m_SpawnRadius;

    ~Particles() {
        glDeleteBuffers(1, &m_PositionBo);
        glDeleteBuffers(1, &m_VelocityBo);
        glDeleteVertexArrays(1, &m_Vao);
    }
    Particles(const Particles& v) = delete;
    Particles& operator=(const Particles& v) = delete;
    Particles(Particles&& o) = delete;
    Particles& operator=(Particles&& o) = delete;
    Particles() = delete;

    Particles(size_t count, Shape shape, float radius):
        m_MaxParticleCount(count),
        m_ParticleCount(0),
        m_Shape(shape),
        m_SpawnRadius(radius)
    {
        glGenBuffers(1, &m_PositionBo);
        glGenBuffers(1, &m_VelocityBo);
        glGenVertexArrays(1, &m_Vao);

        glBindBuffer(GL_COPY_READ_BUFFER,  m_PositionBo);
        glBindBuffer(GL_COPY_WRITE_BUFFER, m_VelocityBo);
        glBufferData(GL_COPY_READ_BUFFER,  m_MaxParticleCount * sizeof(glm::vec4), nullptr, GL_DYNAMIC_COPY);
        glBufferData(GL_COPY_WRITE_BUFFER, m_MaxParticleCount * sizeof(glm::vec4), nullptr, GL_DYNAMIC_COPY);

        // addParticles(count);

        glBindVertexArray(m_Vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_PositionBo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);
    }

    size_t getParticleCount() const { return m_ParticleCount; }
    size_t getMaxParticleCount() const { return m_MaxParticleCount; }

    void addParticles(size_t count, const InstanceData& instanceData) {
        if(m_ParticleCount >= m_MaxParticleCount)
            return;

        count = std::min(count, m_MaxParticleCount - m_ParticleCount);
        assert(m_ParticleCount + count <= m_MaxParticleCount);
        const HostData d(count, m_Shape, m_SpawnRadius, instanceData);

        glBindBuffer(GL_COPY_READ_BUFFER,  m_PositionBo);
        glBindBuffer(GL_COPY_WRITE_BUFFER, m_VelocityBo);
        glBufferSubData(GL_COPY_READ_BUFFER,  m_ParticleCount * sizeof d.pos[0], count * sizeof d.pos[0], d.pos.data());
        glBufferSubData(GL_COPY_WRITE_BUFFER, m_ParticleCount * sizeof d.vel[0], count * sizeof d.vel[0], d.vel.data());

        m_ParticleCount += count;
    }
    void removeParticles(size_t count) {
        count = std::min(count, m_ParticleCount);
        m_ParticleCount -= count;
    }
    void render() const {
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT); // Barrier for compute shader that updates particles
        glBindVertexArray(m_Vao);
        glDrawArrays(GL_POINTS, 0, m_ParticleCount);
    }
    void render(const GLParticlesRenderingProgram& prog, const glmlv::Camera& camera, const InstanceData& i) const {
        prog.setUniformModelViewProjMatrix(
            // camera.getProjMatrix() * camera.getViewMatrix() * i.getModelMatrix()
            camera.getProjMatrix() * camera.getViewMatrix()
        );
        prog.setUniformPointSize(i.pointSize);
        prog.setUniformZScale(i.zScale);
        prog.setUniformZInfluence(i.zInfluence);
        prog.setUniformInnerColor(i.innerColor);
        prog.setUniformOuterColor(i.outerColor);
        render();
    }
    void update(const GLParticlesSimulationProgram& prog, float dt, const InstanceData& i) {
        prog.setUniformParticleCount(m_ParticleCount);
        prog.setUniformDeltaTime(dt);
        prog.setUniformVelMultiplier(i.velMultiplier);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_PositionBo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_VelocityBo);
        glDispatchCompute(1 + m_ParticleCount / 1024, 1, 1);
    }
};

class GLCubeMapProgram: public glmlv::GLProgram {
    const GLint m_UniformModelViewProjMatrixLocation = -1;
    const GLint m_UniformCubeMapLocation             = -1;
public:
    GLCubeMapProgram(const glmlv::fs::path& vs, const glmlv::fs::path& fs):
        GLProgram(glmlv::compileProgram({ vs.string(), fs.string() })),
        m_UniformModelViewProjMatrixLocation(getUniformLocation("uModelViewProjMatrix")),
        m_UniformCubeMapLocation(getUniformLocation("uCubeMap"))
        {}
    void setUniformModelViewProjMatrix(const glm::mat4& m) const { glUniformMatrix4fv(m_UniformModelViewProjMatrixLocation, 1, GL_FALSE, &m[0][0]); }
    void setUniformCubeMap(GLuint i) const { glUniform1i(m_UniformCubeMapLocation, i); }
};

class PostFX_ComputePassProgram: public glmlv::GLProgram {
    GLint m_UniformInputImageLocation            = -1;
    GLint m_UniformOutputImageLocation           = -1;
    GLint m_UniformGammaExponentLocation         = -1;
    GLint m_UniformFinalTouchMulLocation         = -1;
    GLint m_UniformFinalTouchAddLocation         = -1;
    GLint m_UniformGlitchLocation                = -1;
	GLint m_UniformTimeLocation                  = -1;

public:
    PostFX_ComputePassProgram(const glmlv::fs::path& cs):
        GLProgram(glmlv::compileProgram({ cs.string() })),
        m_UniformInputImageLocation           (getUniformLocation("uInputImage")),
        m_UniformOutputImageLocation          (getUniformLocation("uOutputImage")),
        m_UniformGammaExponentLocation        (getUniformLocation("uGammaExponent")),
        m_UniformFinalTouchMulLocation        (getUniformLocation("uFinalTouchMul")),
        m_UniformFinalTouchAddLocation        (getUniformLocation("uFinalTouchAdd")),
        m_UniformGlitchLocation               (getUniformLocation("uGlitch")),
		m_UniformTimeLocation                 (getUniformLocation("uTime"))
        {}
    void setUniformInputImage(GLint i)               const { glUniform1i(m_UniformInputImageLocation, i); }
    void setUniformOutputImage(GLint i)              const { glUniform1i(m_UniformOutputImageLocation, i); }
    void setUniformGammaExponent(GLfloat f)          const { glUniform1f(m_UniformGammaExponentLocation, f); }
    void setUniformFinalTouchMul(const glm::vec3 &v) const { glUniform3fv(m_UniformFinalTouchMulLocation, 1, &v[0]); }
    void setUniformFinalTouchAdd(const glm::vec3 &v) const { glUniform3fv(m_UniformFinalTouchAddLocation, 1, &v[0]); }
    void setUniformGlitch(GLfloat f)                 const { glUniform1f(m_UniformGlitchLocation, f); }
	void setUniformTime(GLfloat f)                   const { glUniform1f(m_UniformTimeLocation, f); }
};


class PostFX_FragmentPassProgram: public glmlv::GLProgram {
    GLint m_UniformHiResTextureLocation          = -1;
    GLint m_UniformLoResTextureLocation          = -1;
    GLint m_UniformWindowSizeLocation            = -1;
    GLint m_UniformBlurTechniqueLocation         = -1;
    GLint m_UniformBoxBlurMatrixHalfSideLocation = -1;
    GLint m_UniformRadialBlurNumSamplesLocation  = -1;
    GLint m_UniformRadialBlurMaxLengthLocation   = -1;
    GLint m_UniformBloomEnabledLocation          = -1;
    GLint m_UniformBloomMatrixHalfSideLocation   = -1;
    GLint m_UniformBloomTexelSkipLocation        = -1;
    GLint m_UniformBloomThresholdLocation        = -1;

public:
    PostFX_FragmentPassProgram(const glmlv::fs::path& vs, const glmlv::fs::path& fs):
        GLProgram(glmlv::compileProgram({ vs.string(), fs.string() })),
        m_UniformHiResTextureLocation         (getUniformLocation("uHiResTexture")),
        m_UniformLoResTextureLocation         (getUniformLocation("uLoResTexture")),
        m_UniformWindowSizeLocation           (getUniformLocation("uWindowSize")),
        m_UniformBlurTechniqueLocation        (getUniformLocation("uBlurTechnique")),
        m_UniformBoxBlurMatrixHalfSideLocation(getUniformLocation("uBoxBlurMatrixHalfSide")),
        m_UniformRadialBlurNumSamplesLocation (getUniformLocation("uRadialBlurNumSamples")),
        m_UniformRadialBlurMaxLengthLocation  (getUniformLocation("uRadialBlurMaxLength")),
        m_UniformBloomEnabledLocation         (getUniformLocation("uBloomEnabled")),
        m_UniformBloomMatrixHalfSideLocation  (getUniformLocation("uBloomMatrixHalfSide")),
        m_UniformBloomTexelSkipLocation       (getUniformLocation("uBloomTexelSkip")),
        m_UniformBloomThresholdLocation       (getUniformLocation("uBloomThreshold"))
        {}
    static const GLuint BLUR_NONE = 1;
    static const GLuint BLUR_BOX = 2;
    static const GLuint BLUR_RADIAL = 3;
    void setUniformHiResTexture(GLint i)          const { glUniform1i(m_UniformHiResTextureLocation, i); }
    void setUniformLoResTexture(GLint i)          const { glUniform1i(m_UniformLoResTextureLocation, i); }
    void setUniformWindowSize(GLuint w, GLuint h) const { glUniform2ui(m_UniformWindowSizeLocation, w, h); }
    void setUniformBlurTechnique(GLuint tech)     const { glUniform1ui(m_UniformBlurTechniqueLocation, tech); }
    void setUniformBoxBlurMatrixHalfSide(GLint i) const { glUniform1i(m_UniformBoxBlurMatrixHalfSideLocation, i); }
    void setUniformRadialBlurNumSamples(GLuint n) const { glUniform1ui(m_UniformRadialBlurNumSamplesLocation, n); }
    void setUniformRadialBlurMaxLength(GLfloat f) const { glUniform1f(m_UniformRadialBlurMaxLengthLocation, f); }
    void setUniformBloomEnabled(bool b)           const { glUniform1i(m_UniformBloomEnabledLocation, b); }
    void setUniformBloomMatrixHalfSide(GLint i)   const { glUniform1i(m_UniformBloomMatrixHalfSideLocation, i); }
    void setUniformBloomTexelSkip(GLfloat f)      const { glUniform1f(m_UniformBloomTexelSkipLocation, f); }
    void setUniformBloomThreshold(GLfloat f)      const { glUniform1f(m_UniformBloomThresholdLocation, f); }
};


enum GBufferTexIndex
{
    GPosition = 0,
    GNormal,
    GAmbient,
    GDiffuse,
    GGlossyShininess,
    GDepth, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
    GBufferTextureCount
};

struct Paths {
    const glmlv::fs::path m_App;
    const std::string     m_AppName;
    const glmlv::fs::path m_AssetsRoot, m_ShadersRoot;
    const glmlv::fs::path m_AppAssets, m_AppShaders;

    Paths(glmlv::fs::path&& appPath):
        m_App(appPath),
        m_AppName     { m_App.stem().string() },
        m_AssetsRoot  { m_App.parent_path() / "assets" },
        m_ShadersRoot { m_App.parent_path() / "shaders" },
        m_AppAssets   { m_AssetsRoot / m_AppName },
        m_AppShaders  { m_ShadersRoot / m_AppName }
        {}
};

template<typename T> struct Rect {
    T x, y, w, h;
    Rect(T x, T y, T w, T h) : x(x), y(y), w(w), h(h) {}
    Rect<float> dividedBySize(float sw, float sh) const {
        return Rect<float>(x / sw, y / sh, w / sw, h / sh);
    }
};

struct Sprites {
    enum SpriteIndex {
        SprImacLogo = 0,
        SprYoanLecoq,
        SprCoralieGoldbaum,
        SprTeacher,
        SprSoundtrack,
        SprRevolve,
        SprCount
    };
    enum AtlasIndex {
        AtlasImacLogo = 0,
        AtlasTexts,
        AtlasCount
    };

    static const std::array<Rect<uint32_t>, AtlasCount> ATLAS_SIZE;
    static const std::array<Rect<uint32_t>, SprCount> SPR_TEXCOORDS_UINT;
    static const std::array<Rect<float>, SprCount> SPR_TEXCOORDS;
    static const std::array<AtlasIndex, SprCount> SPR_ATLAS_INDEX;
    const std::array<glmlv::GLTexture2D, AtlasCount> m_AtlasTexture;
    std::array<glm::vec2, SprCount> m_SprPosition;
    std::array<float, SprCount> m_SprScale;
    std::array<float, SprCount> m_SprAlpha;

    const GLScreenQuadProgram m_Program;
    GLuint m_Vbo, m_Vao;

    ~Sprites() {
        glDeleteVertexArrays(1, &m_Vao);
        glDeleteBuffers(1, &m_Vbo);
    }

    Sprites() = delete;
    Sprites(const Paths& paths):
        m_AtlasTexture {
            paths.m_AppAssets / "images" / "imac_gris_numerique_petit.png",
            paths.m_AppAssets / "images" / "texts.png"
        },
        m_SprPosition {
            glm::vec2(0),
            glm::vec2(0),
            glm::vec2(0),
            glm::vec2(0),
            glm::vec2(0),
            glm::vec2(0)
        },
        m_SprScale { 1, 1, 1, 1, 1, 1 },
        m_SprAlpha { 0, 0, 0, 0, 0, 0 },
        m_Program(
            paths.m_AppShaders / "screenQuad.vs.glsl",
            paths.m_AppShaders / "screenQuad.fs.glsl"
        ),
        m_Vbo(0),
        m_Vao(0)
    {
        static const float STRIP[] = {
            // x, y, u, v
            -1, +1, 0, 0,
            -1, -1, 0, 1,
            +1, +1, 1, 0,
            +1, -1, 1, 1
        };
        glGenBuffers(1, &m_Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof STRIP, STRIP, GL_STATIC_DRAW);

        glGenVertexArrays(1, &m_Vao);
        glBindVertexArray(m_Vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, (void*)8);
    }

    void render(size_t w, size_t h) const {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        for(int i=0 ; i<AtlasCount ; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            m_AtlasTexture[i].bind();
        }
        m_Program.use();
        glBindVertexArray(m_Vao);
        for(int i=0 ; i<SprCount ; ++i) {
            const auto& r = SPR_TEXCOORDS_UINT[i];
            const auto imgScale = glm::vec2(r.w / float(w), r.h / float(h));
            const auto& tc = SPR_TEXCOORDS[i];
            m_Program.setUniformTexture(SPR_ATLAS_INDEX[i]);
            m_Program.setUniformScale(m_SprScale[i] * imgScale);
            m_Program.setUniformPosition(m_SprPosition[i]);
            m_Program.setUniformTexCoordsOffset(glm::vec2(tc.x, tc.y));
            m_Program.setUniformTexCoordsSize(glm::vec2(tc.w, tc.h));
            m_Program.setUniformAlphaMultipler(m_SprAlpha[i]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
};


struct ParticlesManager {
    GLParticlesSimulationProgram m_SimulationProgram;
    GLParticlesRenderingProgram m_RenderingProgram;
    Particles m_LeftReactorParticles;
    Particles m_RightReactorParticles;
    Particles::InstanceData m_LeftReactorParticlesInstanceData;
    Particles::InstanceData m_RightReactorParticlesInstanceData;

    ParticlesManager(const Paths& paths): 
        m_SimulationProgram(paths.m_AppShaders / "particles.cs.glsl"),
        m_RenderingProgram(
            paths.m_AppShaders / "particles.vs.glsl",
            paths.m_AppShaders / "particles.fs.glsl"
        ),
        // NOTE: Do NOT set first argument to zero: It's the max number of particles
        // and is set once.
		m_LeftReactorParticles(1<<17, Particles::Shape::Disk, 321.f),
		m_RightReactorParticles(1<<17, Particles::Shape::Disk, 321.f)
    {
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glEnable(GL_PROGRAM_POINT_SIZE);
		//glEnable(34913 /*GL_POINT_SPRITE*/);
    }
    void render(const glmlv::Camera& cam) const {
        m_RenderingProgram.use();
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        m_LeftReactorParticles.render(m_RenderingProgram, cam, m_LeftReactorParticlesInstanceData);
        m_RightReactorParticles.render(m_RenderingProgram, cam, m_RightReactorParticlesInstanceData);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
    void update(float dt) {
        m_SimulationProgram.use();
        m_LeftReactorParticles.update(m_SimulationProgram, dt, m_LeftReactorParticlesInstanceData);
        m_RightReactorParticles.update(m_SimulationProgram, dt, m_RightReactorParticlesInstanceData);
    }
};



class SkyboxCubeMesh {
    GLuint m_Vbo, m_Vao;
public:
    ~SkyboxCubeMesh() {
        glDeleteVertexArrays(1, &m_Vao);
        glDeleteBuffers(1, &m_Vbo);
    }
    static constexpr size_t VERTEX_COUNT = 14;
    SkyboxCubeMesh() {
        static const float STRIP[VERTEX_COUNT*3] = {
            -1.f,  1.f,  1.f, // Front-top-left
             1.f,  1.f,  1.f, // Front-top-right
            -1.f, -1.f,  1.f, // Front-bottom-left
             1.f, -1.f,  1.f, // Front-bottom-right
             1.f, -1.f, -1.f, // Back-bottom-right
             1.f,  1.f,  1.f, // Front-top-right
             1.f,  1.f, -1.f, // Back-top-right
            -1.f,  1.f,  1.f, // Front-top-left
            -1.f,  1.f, -1.f, // Back-top-left
            -1.f, -1.f,  1.f, // Front-bottom-left
            -1.f, -1.f, -1.f, // Back-bottom-left
             1.f, -1.f, -1.f, // Back-bottom-right
            -1.f,  1.f, -1.f, // Back-top-left
             1.f,  1.f, -1.f  // Back-top-right
        };
        glGenBuffers(1, &m_Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof STRIP, STRIP, GL_STATIC_DRAW);

        glGenVertexArrays(1, &m_Vao);
        glBindVertexArray(m_Vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    }
    void render() const {
        glBindVertexArray(m_Vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, VERTEX_COUNT);
    }
};

class Skybox {
    GLCubeMapProgram        m_Program;
    glmlv::GLSampler        m_Sampler;
    SkyboxCubeMesh          m_CubeMesh;
public:
    // TODO(coraliegold) Add new skybox cubemaps here, and improve constructor accordingly
    enum Sky {
        SkySimpleColorTest = 0,
        SkySpaceKurt,
        SkySpaceUlukaiCorona,
        SkySpaceUlukaiRedEclipse,
		SkyPlanetFlashBack,
		SkySpace,
		SkyCount,
    };

    float                   m_Scale;
    glmlv::GLCubeMapTexture m_Skies[SkyCount];
    int                     m_CurrentSky; // int, so it is editable via radiobutton


    Skybox(const Paths& paths, float scale): 
        m_Program(
            paths.m_AppShaders / "cubeMap.vs.glsl",
            paths.m_AppShaders / "cubeMap.fs.glsl"
        ),
        m_Sampler(glmlv::GLSamplerParams().withWrapST(GL_CLAMP_TO_BORDER).withMinMagFilter(GL_LINEAR)),
        m_CubeMesh(),
        m_Scale(scale),
        m_Skies{},
        m_CurrentSky(SkySpace)
    {
        glmlv::CubeMapFaceImages faces;

        faces.px = glmlv::Image2DRGBA(128, 128, 255, 000, 000, 255);
        faces.py = glmlv::Image2DRGBA(128, 128, 000, 255, 000, 255);
        faces.pz = glmlv::Image2DRGBA(128, 128, 000, 000, 255, 255);
        faces.nx = glmlv::Image2DRGBA(128, 128, 128, 000, 000, 255);
        faces.ny = glmlv::Image2DRGBA(128, 128, 000, 128, 000, 255);
        faces.nz = glmlv::Image2DRGBA(128, 128, 000, 000, 128, 255);
        m_Skies[SkySimpleColorTest].uploadImages(faces);

        faces.px = readImageNoException(paths.m_AppAssets / "skyboxes" / "kurt" / "space_rt.png");
        faces.py = readImageNoException(paths.m_AppAssets / "skyboxes" / "kurt" / "space_up.png");
        faces.pz = readImageNoException(paths.m_AppAssets / "skyboxes" / "kurt" / "space_bk.png");
        faces.nx = readImageNoException(paths.m_AppAssets / "skyboxes" / "kurt" / "space_lf.png");
        faces.ny = readImageNoException(paths.m_AppAssets / "skyboxes" / "kurt" / "space_dn.png");
        faces.nz = readImageNoException(paths.m_AppAssets / "skyboxes" / "kurt" / "space_ft.png");
        m_Skies[SkySpaceKurt].uploadImages(faces);

        faces.px = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_rt.png");
        faces.py = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_up.png");
        faces.pz = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_bk.png");
        faces.nx = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_lf.png");
        faces.ny = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_dn.png");
        faces.nz = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_ft.png");
        m_Skies[SkySpaceUlukaiCorona].uploadImages(faces);

        faces.px = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_rt.png");
        faces.py = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_up.png");
        faces.pz = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_bk.png");
        faces.nx = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_lf.png");
        faces.ny = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_dn.png");
        faces.nz = readImageNoException(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_ft.png");
        m_Skies[SkySpaceUlukaiRedEclipse].uploadImages(faces);

		faces.px = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_planet" / "morning_rt.tga");
		faces.py = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_planet" / "morning_up.tga");
		faces.pz = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_planet" / "morning_bk.tga");
		faces.nx = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_planet" / "morning_lf.tga");
		faces.ny = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_planet" / "morning_dn.tga");
		faces.nz = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_planet" / "morning_ft.tga");
		m_Skies[SkyPlanetFlashBack].uploadImages(faces);

		faces.px = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_space" / "purplenebula_rt.tga");
		faces.py = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_space" / "purplenebula_up.tga");
		faces.pz = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_space" / "purplenebula_bk.tga");
		faces.nx = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_space" / "purplenebula_lf.tga");
		faces.ny = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_space" / "purplenebula_dn.tga");
		faces.nz = readImageNoException(paths.m_AppAssets / "skyboxes" / "custom_space" / "purplenebula_ft.tga");
		m_Skies[SkySpace].uploadImages(faces);
    }
    void render(const glmlv::Camera& camera) {
        // sqrt(s*s + s*s + s*s) < far
        // sqrt(3 * s^2) < far
        // 3 * s^2 < far^2
        // s^2 < (far^2)/3
        // s < sqrt((far^2)/3)
        const auto cfar = camera.m_Far*0.98f;
        m_Scale = sqrtf(cfar*cfar/3.f);
        const auto modelMatrix = glm::scale(glm::mat4(1.f), glm::vec3(m_Scale));
        auto viewMatrix = camera.getViewMatrix();
        // Cancel camera translation
        viewMatrix[3][0] = 0;
        viewMatrix[3][1] = 0;
        viewMatrix[3][2] = 0;
        const auto mvp = camera.getProjMatrix() * viewMatrix * modelMatrix;
        m_Program.use();
        m_Program.setUniformModelViewProjMatrix(mvp);
        m_Program.setUniformCubeMap(0);
        glActiveTexture(GL_TEXTURE0);
        m_Skies[m_CurrentSky].bind();
        // m_Sampler.bindToTextureUnit(0);
		glDepthMask(GL_FALSE);
        m_CubeMesh.render();
		glDepthMask(GL_TRUE);
    }
};


struct ForwardRendering {
    const glmlv::GLForwardRenderingProgram m_Program;

    ForwardRendering(const Paths& paths):
        m_Program(
            paths.m_AppShaders / "forward.vs.glsl",
            paths.m_AppShaders / "forward.fs.glsl"
        )
        {}
};

struct DeferredRendering {
    const glmlv::GLDeferredGPassProgram       m_GPassProgram;
    const glmlv::GLDeferredShadingPassProgram m_ShadingPassProgram;
    glmlv::GLTexture2D                        m_GBufferTextures[GBufferTextureCount];
    GLuint                                    m_GBufferFbo;

    bool m_GuiDisplaysGBufferTextures;
    int m_GuiGBufferTexIndex;

    DeferredRendering(const Paths& paths, GLsizei w, GLsizei h):
        m_GPassProgram(
            paths.m_AppShaders / "geometryPass.vs.glsl",
            paths.m_AppShaders / "geometryPass.fs.glsl"
        ),
        m_ShadingPassProgram(
            paths.m_AppShaders / "shadingPass.vs.glsl",
            paths.m_AppShaders / "shadingPass.fs.glsl"
        ),
        m_GBufferTextures {
            { GL_RGB32F, w, h },
            { GL_RGB32F, w, h },
            { GL_RGB32F, w, h },
            { GL_RGB32F, w, h },
            { GL_RGBA32F, w, h },
            { GL_DEPTH_COMPONENT32F, w, h }
        },
        m_GBufferFbo(0),
        m_GuiDisplaysGBufferTextures(false),
        m_GuiGBufferTexIndex(GNormal)
    {
        glGenFramebuffers(1, &m_GBufferFbo);
        assert(m_GBufferFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_GBufferFbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBufferTextures[0].glId(), 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBufferTextures[1].glId(), 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBufferTextures[2].glId(), 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_GBufferTextures[3].glId(), 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_GBufferTextures[4].glId(), 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, m_GBufferTextures[5].glId(), 0);
        static const GLenum drawBuffers[5] = {
            GL_COLOR_ATTACHMENT0, 
            GL_COLOR_ATTACHMENT1, 
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4
        };
        glDrawBuffers(5, drawBuffers);
        handleFboStatus(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
};

struct DirectionalShadowMapping {
    int m_Resolution; // NOTE: int to be editable via RadioButton
    const glmlv::GLDirectionalSMProgram m_Program;
    const glmlv::GLDisplayDepthMapProgram m_DisplayDepthMapProgram;
    const glmlv::GLSampler m_Sampler;
    glmlv::GLTexture2D m_Texture;
    GLuint m_Fbo;

    bool m_IsDirty;
    bool m_GuiDisplaysShadowMap;

    DirectionalShadowMapping(const Paths& paths): 
        m_Resolution(512),
        m_Program(
            paths.m_AppShaders / "directionalSM.vs.glsl",
            paths.m_AppShaders / "directionalSM.fs.glsl"
        ),
        m_DisplayDepthMapProgram(
            paths.m_AppShaders / "shadingPass.vs.glsl",
            paths.m_AppShaders / "displayDepth.fs.glsl"
        ),
        m_Sampler(glmlv::GLSamplerParams().withWrapST(GL_CLAMP_TO_BORDER).withMinMagFilter(GL_LINEAR)),
        m_Texture(GL_DEPTH_COMPONENT32F, m_Resolution, m_Resolution),
        m_Fbo(0),
        m_IsDirty(true),
        m_GuiDisplaysShadowMap(false)
    {
        glSamplerParameteri(m_Sampler.glId(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glSamplerParameteri(m_Sampler.glId(), GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glGenFramebuffers(1, &m_Fbo);
        assert(m_Fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_Texture.glId(), 0);
        handleFboStatus(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
};


struct PostFX_TextureFbo {
    const glmlv::GLTexture2D m_Texture;
    GLuint m_Fbo;

    PostFX_TextureFbo(GLsizei w, GLsizei h): 
        m_Texture(GL_RGBA32F, w, h),
        m_Fbo(0)
    {
        glGenFramebuffers(1, &m_Fbo);
        assert(m_Fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture.glId(), 0);
        static const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBuffers);
        handleFboStatus(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
};

struct PostFX_ComputePass {
    const PostFX_ComputePassProgram m_Program;
    const glmlv::GLTexture2D m_InputDepthTexture;
    const PostFX_TextureFbo m_Input, m_Output;
    bool m_IsEnabled;
    float m_Gamma;
    glm::vec3 m_FinalTouchMul;
    glm::vec3 m_FinalTouchAdd;
	float m_Glitch;
    float m_Time;

    PostFX_ComputePass(const Paths& paths, GLsizei w, GLsizei h):
        m_Program(paths.m_AppShaders / "postFX.cs.glsl"),
        m_InputDepthTexture(GL_DEPTH_COMPONENT32F, w, h),
        m_Input(w, h),
        m_Output(w, h),
        m_IsEnabled(false),
        m_Gamma(2.2f),
        m_FinalTouchMul(1.0f),
        m_FinalTouchAdd(0.0f),
        m_Glitch(1.0f),
		m_Time(0.0f)
    {
        // Still attach a depth texture to input FBO for when using Forward Pipeline
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Input.m_Fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_InputDepthTexture.glId(), 0);
        handleFboStatus(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
};
struct PostFX_FragmentPass {
    const GLuint m_LoResWidth, m_LoResHeight;
    const PostFX_FragmentPassProgram m_Program;
    const glmlv::GLTexture2D m_HiResDepthTexture;
    const PostFX_TextureFbo m_HiRes, m_LoRes;
    const glmlv::GLSampler m_LinearSampler, m_NearestSampler;
    bool m_IsEnabled;
    int m_BlurTechnique;
    int m_BoxBlurMatrixHalfSide;
    int m_RadialBlurNumSamples;
    float m_RadialBlurMaxLength;
    bool m_BloomEnabled;
    int m_BloomMatrixHalfSide;
    float m_BloomTexelSkip;
    float m_BloomThreshold;

    PostFX_FragmentPass(const Paths& paths, GLsizei w, GLsizei h):
        m_LoResWidth(w/2),
        m_LoResHeight(h/2),
        m_Program(
            paths.m_AppShaders / "shadingPass.vs.glsl",
            paths.m_AppShaders / "postFX.fs.glsl"
        ),
        m_HiResDepthTexture(GL_DEPTH_COMPONENT32F, w, h),
        m_HiRes(w, h),
        m_LoRes(m_LoResWidth, m_LoResHeight),
        m_LinearSampler(glmlv::GLSamplerParams().withWrapST(GL_CLAMP_TO_BORDER).withMinMagFilter(GL_LINEAR)),
        m_NearestSampler(glmlv::GLSamplerParams().withWrapST(GL_CLAMP_TO_BORDER).withMinMagFilter(GL_NEAREST)),
        m_IsEnabled(false),
        m_BlurTechnique(PostFX_FragmentPassProgram::BLUR_NONE),
        m_BoxBlurMatrixHalfSide(2),
        m_RadialBlurNumSamples(16),
        m_RadialBlurMaxLength(0.25f),
        m_BloomEnabled(true),
        m_BloomMatrixHalfSide(2),
        m_BloomTexelSkip(6),
        m_BloomThreshold(0.5f)
    {
        // Still attach a depth texture to input FBO for when using Forward Pipeline
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_HiRes.m_Fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_HiResDepthTexture.glId(), 0);
        handleFboStatus(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
};

struct PostFX {
    PostFX_FragmentPass m_FragmentPass;
    PostFX_ComputePass m_ComputePass;

    PostFX(const Paths& paths, GLsizei w, GLsizei h):
        m_FragmentPass(paths, w, h),
        m_ComputePass(paths, w, h)
        {}
};

// 0-2:   Soleil. fade-in
// 2-5:   Soleil-base. zoom-in sur la base
// 5-8:   rotation; fade in noms
// 8-11:  fade out noms
// 11-21: zoom jusqu'à très proche du vaisseau
// 21-28: fade in REVOLVE
// 28-30: Flash lumière
// 30-33: Fissures
// 33-36: Flashback
// 36-39: Plan rotation autour du vaisseau
// 39-42: Focus sur les débris. La base s'écroule
// 42-45: Flashback
// 45-49: Vaisseau-base. Il reste plus grand chose de la base.
// 49-53: Flashback
// 53-55: Allumage des réacteurs
// 55-59: Décollage vaisseau
// 59-63: Plan base-soleil, vaisseau s'éloigne
// 63-70: Base s'écroule totalement
// 70-88: Vue à partir du vaisseau, puis fondu en blanc.
//        Soleil prend moitié droite de l'écran. Base prend sur la gauche.

#define PIPELINE_FORWARD 1
#define PIPELINE_DEFERRED 2

#define SCENEID_END_OF_THE_WORLD 1
#define SCENEID_CITY 2


class Story {

    struct Interpolations {
        static glm::vec3 slerp(const glm::vec3& a, const glm::vec3& b, float t) {
#if 1
            const auto al = glm::length(a);
            const auto bl = glm::length(b);
            const auto an = a / al;
            const auto bn = b / bl;
            const auto xn = glm::slerp(an, bn, t);
            const auto x = xn * glm::mix(al, bl, t);
#else
            const auto x = glm::slerp(a, b, t);
#endif
            if(glm::any(glm::isnan(x))) {
                std::clog << "WARN: slerp(" << a << ", " << b << ", " << t << ") returned " << x << std::endl;
            }
            return x;
        }
        static glm::vec3 lerp3(const glm::vec3& a, const glm::vec3& b, float t) { return glm::mix(a, b, t); }
        static glm::vec2 lerp2(const glm::vec2& a, const glm::vec2& b, float t) { return glm::mix(a, b, t); }
        static float lerp(const float& a, const float& b, float t) { return glm::mix(a, b, t); }
        template<typename T>
        static T lower(const T& x, const T&, float) { return x; }
        template<typename T>
        static T upper(const T&, const T& x, float) { return x; }
        template<typename T>
        static T nearest(const T& a, const T& b, float t) { return t < 0.5f ? a : b; }
    };

    template<typename T>
    struct Timeline {
        typedef std::function<T(const T&, const T&, float)> Lerp;
        const Lerp m_Lerp;
        std::map<float, T> m_Keyframes;

        Timeline() = delete;
        Timeline(const Lerp& lerp, std::map<float, T>&& init):
            m_Lerp(lerp),
            m_Keyframes(init)
            {}

        T at(float t) const {
            assert(!m_Keyframes.empty() && "There must be at least one keyframe!");

            auto upper = m_Keyframes.upper_bound(t);
            if(upper == m_Keyframes.cend())
                --upper;

            auto lower = upper;
            if(lower != m_Keyframes.begin())
                --lower;

            if(t <= lower->first)
                return lower->second;
            if(t >= upper->first)
                return upper->second;

            const auto lerp_factor = (t - lower->first) / (upper->first - lower->first);
            return m_Lerp(lower->second, upper->second, lerp_factor);
        }
    };

    bool m_IsPlaying;
    bool m_IsPlayKeyHeld;
    bool m_ShouldGuiBeVisible;
    bool m_IsGuiKeyHeld;
    float m_PlayheadTime; // time (seconds) since entering demo mode.

public:
    const glmlv::fs::path m_SoundtrackWavPath;

    // TODO(coraliegold): Add new timelines as needed.
    // Every property that needs to change over time needs a timeline.
    /*const Timeline<glmlv::Camera::Mode> m_CameraMode;
    const Timeline<glm::vec3> m_CameraForward;
    const Timeline<glm::vec2> m_CameraNoiseFactor;
    const Timeline<float> m_CameraNoiseSpeed;
    const Timeline<float> m_CameraFovY;*/

	const Timeline<int> m_Pipeline;

	const Timeline<float> m_SpritesYoanLecoqAlpha;
	const Timeline<glm::vec2> m_SpritesYoanLecoqPos;
	const Timeline<float> m_SpritesCoralieGoldbaumAlpha;
	const Timeline<glm::vec2> m_SpritesCoralieGoldbaumPos;
	const Timeline<float> m_SpritesRevolveAlpha;
	const Timeline<float> m_SpritesTeacherAlpha;
	const Timeline<glm::vec2> m_SpritesTeacherPos;
	const Timeline<float> m_SpritesTeacherScale;
	const Timeline<float> m_SpritesSoundtrackAlpha;
	const Timeline<glm::vec2> m_SpritesSoundtrackPos;
	const Timeline<float> m_SpritesSoundtrackScale;
	const Timeline<float> m_SpritesIMACAlpha;

	const Timeline<bool> m_ComputePass;
	const Timeline<float> m_ComputePassGamma;
	const Timeline<glm::vec3> m_ComputePassFinalTouchAdd;
	const Timeline<glm::vec3> m_ComputePassFinalTouchMul;

	const Timeline<bool> m_FragmentPass;
	const Timeline<int> m_BlurKind;
	const Timeline<int> m_BlurHalfSide;
	const Timeline<bool> m_Bloom;
	const Timeline<float> m_BloomHalfSide;
	const Timeline<float> m_BloomTexelSkip;
	const Timeline<float> m_BloomThreshold;

	const Timeline<glmlv::Camera::Mode> m_CameraMode;
	const Timeline<glm::vec3> m_CameraTarget;
	const Timeline<glm::vec3> m_CameraForward;
	const Timeline<glm::vec2> m_CameraNoiseFactor;
	const Timeline<float> m_CameraNoiseSpeed;
	const Timeline<float> m_CameraVerticalFOV;
	const Timeline<glm::vec3> m_CameraFreeflyPosition;
	const Timeline<glm::vec3> m_CameraFreeflyForward;

	const Timeline<Skybox::Sky> m_Skybox;
	const Timeline<int> m_SceneID;

    static constexpr float BPM = 170;
    static constexpr float DURATION = 88; // 1 min 28
    static constexpr int PLAY_KEY = GLFW_KEY_SPACE;
    static constexpr int GUI_KEY = GLFW_KEY_G;

    Story(const Paths& paths):
        m_IsPlaying(false),
        m_IsPlayKeyHeld(false),
        m_ShouldGuiBeVisible(true),
        m_IsGuiKeyHeld(false),
        m_PlayheadTime(0),
        m_SoundtrackWavPath(paths.m_AppAssets / "music" / "outsider.wav"),
        // TODO(coraliegold): Here, keyframes are specified for each timeline.
        // It's backed by a std::map so one can also run some logic in the constructor.
		

		/****** Pipeline ******/


		m_Pipeline(Interpolations::lower<int>, {
			// Plan 1
			{ 0, PIPELINE_DEFERRED },
			// Plan 2
			{ 4.5, PIPELINE_FORWARD },
		}),


		/****** Sprites ******/
		

		m_SpritesYoanLecoqAlpha(Interpolations::lerp, {
			// Plan 1
			{ 0, 0 },
			// Plan 2
			{ 4.5, 0 }, 
			{ 8, 1 },
			// Plan 3
			{ 9.5, 0 },
		}),
		m_SpritesYoanLecoqPos(Interpolations::lower<glm::vec2>, {
			{ 0, glm::vec2(-0.650, 0.800) },
		}),

		m_SpritesCoralieGoldbaumAlpha(Interpolations::lerp, {
			// Plan 1
			{ 0, 0 },
			// Plan 2
			{ 4.5, 0 },
			{ 8, 1 },
			// Plan 3
			{ 9.5, 0 },
		}),
		m_SpritesCoralieGoldbaumPos(Interpolations::lower<glm::vec2>, {
			{ 0, glm::vec2(0.500, -0.800) },
		}),

		m_SpritesRevolveAlpha(Interpolations::lerp, {
			{ 0, 0 },
			// Plan 5
			{ 21, 0 },
			{ 25, 1 },
			{ 28.6, 0 },
		}),

		m_SpritesTeacherAlpha(Interpolations::lerp, {
			// Plan 1
			{ 0, 0 },
			// Plan 4
			{ 15, 0 },
			{ 18, 1 },
			// Plan 5
			{ 19.5, 0 },
		}),
		m_SpritesTeacherPos(Interpolations::lower<glm::vec2>, {
			{ 0, glm::vec2(0.750, 0.800) },
		}),
		m_SpritesTeacherScale(Interpolations::lerp, {
			{ 0, 1.5 },
		}),

		m_SpritesSoundtrackAlpha(Interpolations::lerp, {
			// Plan 1
			{ 0, 0 },
			// Plan 4
			{ 15, 0 },
			{ 18, 1 },
			// Plan 5
			{ 19.5, 0 },
		}),
		m_SpritesSoundtrackPos(Interpolations::lower<glm::vec2>, {
			{ 0, glm::vec2(-0.350, -0.800) },
		}),
		m_SpritesSoundtrackScale(Interpolations::lerp, {
			{ 0, 1.5 },
		}),

		m_SpritesIMACAlpha(Interpolations::lerp, {
		// Plan 1
			{ 0, 0 },
			// Plan 19
			{ 84, 0 },
			{ 87, 1 },
		}),


		/****** Compute Pass *****/


		m_ComputePass(Interpolations::lower<bool>, {
			// Plan 1
			{ 0, true },
		}),
		m_ComputePassGamma(Interpolations::lerp, {
			// Plan 1
			{ 0, 0 },
			{ 2, 1 },
			// Plan 6
			{ 28.6, 1 },
			{ 28.7, 16 },
			{ 29.0, 1 },
			// Plan 19
			{ 82, 1 },
			{ 90, 0 },
		}),
		m_ComputePassFinalTouchAdd(Interpolations::lerp3, {
			// Plan 1
			{ 0, glm::vec3(0) },
			{ 2, glm::vec3(0) },
			// Plan 6
			{ 28.6, glm::vec3(0) },
			{ 28.7, glm::vec3(2) },
			{ 28.8, glm::vec3(0) },
		}),
		m_ComputePassFinalTouchMul(Interpolations::lerp3, {
			// Plan 1
			{ 0, glm::vec3(-2) },
			{ 2, glm::vec3(1) },
			// Plan 6
			{ 28.6, glm::vec3(1) },
			{ 28.7, glm::vec3(2) },
			{ 28.8, glm::vec3(1) },
			// Plan 19
			{ 82, glm::vec3(1) },
			{ 90, glm::vec3(-2) },
		}),


		/*** Fragment Pass ***/


		m_FragmentPass(Interpolations::lower<bool>, {
			// Plan 1
			{ 0, false },
			// Plan 8
			{ 33, true },
			// Plan 9
			{ 36, false },
			// Plan 11
			{ 42, true },
			// Plan 12
			{ 45, false },
			// Plan 13
			{ 49, true },
			// Plan 14
			{ 53, false },
		}),

		m_BlurKind(Interpolations::lower<int>, {
			// Plan 1
			{ 0, 1 },
		}),
		m_BlurHalfSide(Interpolations::lower<int>, {
			// Plan 1
			{ 0, 3 },
		}),

		m_Bloom(Interpolations::lower<bool>, {
			// Plan 1
			{ 0, false },
			// Plan 8
			{ 33, true },
			// Plan 9
			{ 36, false },
			// Plan 11
			{ 42, true },
			// Plan 12
			{ 45, false },
			// Plan 13
			{ 49, true },
			// Plan 14
			{ 53, false },
		}),
		m_BloomHalfSide(Interpolations::lower<int>, {
			// Plan 1
			{ 0, 3 },
		}),
		m_BloomTexelSkip(Interpolations::lower<float>, {
			// Plan 1
			{ 0, 5.5 },
		}),
		m_BloomThreshold(Interpolations::lower<float>, {
			// Plan 1
			{ 0, 0.125 },
		}),


		/****** Camera ******/


		m_CameraMode(Interpolations::lower<glmlv::Camera::Mode>, {
			// Plan 1
			{ 0, glmlv::Camera::Mode::LookAt },
			// Plan 8
			{ 33, glmlv::Camera::Mode::FreeFly },
			// Plan 9
			{ 36.0, glmlv::Camera::Mode::LookAt },
			// Plan 11
			{ 42.0, glmlv::Camera::Mode::FreeFly },
			// Plan 12
			{ 45, glmlv::Camera::Mode::LookAt },
			// Plan 13
			{ 49, glmlv::Camera::Mode::FreeFly },
			// Plan 14
			{ 53, glmlv::Camera::Mode::LookAt },
		}),
		m_CameraTarget(Interpolations::lower<glm::vec3>, {
			// Plan 1
			{ 0, glm::vec3(0, 0, -1) },
			// Plan 18
			{ 69.99, glm::vec3(0, 0, -1) },
			{ 70, glm::vec3(0, 0, -300) },
		}),
		m_CameraForward(Interpolations::lerp3, {
			// Plan 1
			{ 0, glm::vec3(0, 0, -1750) },
			// Plan 2
			{ 4.5, glm::vec3(-25, 0, -1200) },
			// Plan 3
			{ 8, glm::vec3(-50, 0, -500) },
			// Plan 4
			{ 11, glm::vec3(-5, 0, -100) },
			// Plan 5
			{ 21, glm::vec3(-5, -5, -5) },
			// Plan 6
			{ 28.7, glm::vec3(-3, 0, -1.5) },
			// Plan 7
			{ 28.81, glm::vec3(-3, 0, -1.5) },
			{ 29, glm::vec3(-25, 15, -35) },
			{ 30, glm::vec3(-15, 15, -35) },
			{ 31, glm::vec3(-5, 15, -35) },
			{ 32, glm::vec3(5, 15, -35) },
			{ 33, glm::vec3(15, 15, -35) },
			// Plan 8
			
			// Plan 9
			{ 35.99, glm::vec3(15, 15, -35) },
			{ 36, glm::vec3(-35, -5, -15) },
			{ 39, glm::vec3(40, -5, -5) },
			// Plan 10

			// Plan 11

			// Plan 12

			// Plan 13

			// Plan 14
			{ 52.99, glm::vec3(40, -5, -5) },
			{ 53, glm::vec3(0, -0.5, -3) },

			// Plan 15
			{ 54.99, glm::vec3(0, -0.5, -3) },
			{ 55, glm::vec3(-4.5, -0.5, -2) },

			// Plan 16
			{ 58.99, glm::vec3(-4.5, -0.5, -2) },
			{ 59, glm::vec3(20, -13, -60) },

			// Plan 17
			{ 62.99, glm::vec3(20, -13, -60) },
			{ 63, glm::vec3(1, 5, -90) },

			// Plan 18
			{ 69.99, glm::vec3(1, 5, -90) },
			{ 70, glm::vec3(-500, -60, -200) },
			{ 76, glm::vec3(-500, -60, 100) },

			// Plan 19
			{ 82, glm::vec3(-500, -60, 220) },
			{ 90, glm::vec3(-500, -60, 500) },

		}),
		m_CameraNoiseFactor(Interpolations::lower<glm::vec2>, {
			// Plan 1
			{ 0, glm::vec2(0, 0) },
			// Plan 10
			{ 38.99, glm::vec2(0, 0) },
			{ 39, glm::vec2(0.1, 0.1) },
			// Plan 11
			{ 41.99, glm::vec2(0, 0) },
			// Plan 12
			{ 44.99, glm::vec2(0, 0) },
			{ 45, glm::vec2(0.1, 0.1) },
			// Plan 13
			{ 49, glm::vec2(0, 0) },
			// Plan 14
			{ 52.99, glm::vec2(0, 0) },
			{ 53, glm::vec2(0.1, 0.1) },
			{ 53.5, glm::vec2(0.2, 0.2) },
			// Plan 15
			{ 54, glm::vec2(0.1, 0.1) },
			// Plan 17
			{ 62.99, glm::vec2(0, 0) },
			{ 63, glm::vec2(0.1, 0.1) },
			{ 70, glm::vec2(0, 0) },

		}),
		m_CameraNoiseSpeed(Interpolations::lerp, {
			// Plan 1
			{ 0, 20. },
			}),
		m_CameraVerticalFOV(Interpolations::lerp, {
			// Plan 1
			{ 0, 3.124 },
			// Plan 2
			{ 5.2, 1 },
		}),
		m_CameraFreeflyPosition(Interpolations::lower<glm::vec3>, {
			// Plan 1
			{ 0, glm::vec3(0, 0, 0) },
			// Plan 8
			{ 32.99, glm::vec3(0, 0, 0) },
			{ 33, glm::vec3(0, 0, 0) },
			// Plan 9
			{ 35.99, glm::vec3(0, 0, 0) },
			{ 36.0, glm::vec3(0, 0, 0) },
			// Plan 11
			{ 41.99, glm::vec3(0, 0, 0) },
			{ 42.0, glm::vec3(0, 0, 0) },
			// Plan 12
			{ 44.99, glm::vec3(0, 0, 0) },
			{ 45, glm::vec3(0, 0, 0) },
			// Plan 13
			{ 48.99, glm::vec3(0, 0, 0) },
			{ 49, glm::vec3(0, 0, 0) },
			// Plan 14
			{ 52.99, glm::vec3(0, 0, 0) },
			{ 53, glm::vec3(0, 0, 0) },
		}),
		m_CameraFreeflyForward(Interpolations::lower<glm::vec3>, {
			// Plan 1
			{ 0, glm::vec3(0, 0, 0) },
			// Plan 8
			{ 32.99, glm::vec3(0, 0, 0) },
			{ 33, glm::vec3(0, 0, 0) },
			// Plan 9
			{ 35.99, glm::vec3(0, 0, 0) },
			{ 36.0, glm::vec3(0, 0, 0) },
			// Plan 11
			{ 41.99, glm::vec3(0, 0, 0) },
			{ 42.0, glm::vec3(0, 0, 0) },
			// Plan 12
			{ 44.99, glm::vec3(0, 0, 0) },
			{ 45, glm::vec3(0, 0, 0) },
			// Plan 13
			{ 48.99, glm::vec3(0, 0, 0) },
			{ 49, glm::vec3(0, 0, 0) },
			// Plan 14
			{ 52.99, glm::vec3(0, 0, 0) },
			{ 53, glm::vec3(0, 0, 0) },
		}),


		/****** Skybox ******/


		m_Skybox(Interpolations::lower<Skybox::Sky>, {
			// Plan 1
			{ 0, Skybox::Sky::SkySpace },
			// Plan 8
			{ 33, Skybox::Sky::SkyPlanetFlashBack },
			// Plan 9
			{ 36, Skybox::Sky::SkySpace },
			// Plan 11
			{ 42, Skybox::Sky::SkyPlanetFlashBack },
			// Plan 12
			{ 45, Skybox::Sky::SkySpace },
			// Plan 13
			{ 49, Skybox::Sky::SkyPlanetFlashBack },
			// Plan 14
			{ 53, Skybox::Sky::SkySpace },
		}),

		/****** SceneID ******/

		m_SceneID(Interpolations::lower<int>, {
			// Plan 1
			{ 0, SCENEID_END_OF_THE_WORLD },
			// Plan 8
			{ 33, SCENEID_CITY },
			// Plan 9
			{ 36, SCENEID_END_OF_THE_WORLD },
			// Plan 11
			{ 42, SCENEID_CITY },
			// Plan 12
			{ 45, SCENEID_END_OF_THE_WORLD },
			// Plan 13
			{ 49, SCENEID_CITY },
			// Plan 14
			{ 53, SCENEID_END_OF_THE_WORLD },
		})

        {}
		

    bool isPlaying() const { return m_IsPlaying; }
    float getPlayheadTime() const { return m_PlayheadTime; }
    bool shouldGuiBeVisible() const { return m_ShouldGuiBeVisible; }

    void play() {
        if(m_IsPlaying) {
            std::clog << "WARN: Trying to enter demo mode, but m_IsPlaying is already true!" << std::endl;
            return;
        }

        m_PlayheadTime = 0;
        m_IsPlaying = true;
        m_ShouldGuiBeVisible = false;
        glmlv::GlobalWavPlayer::playWav(m_SoundtrackWavPath);
    }

    void stop() {
        if(!m_IsPlaying) {
            std::clog << "WARN: Trying to leave demo mode, but m_IsPlaying is already false!" << std::endl;
            return;
        }

        m_IsPlaying = false;
        m_ShouldGuiBeVisible = true;
        glmlv::GlobalWavPlayer::stopAll();
    }

    void togglePlayStop() {
        if(m_IsPlaying) stop(); else play();
    }

    void update(glmlv::GLFWHandle& glfwHandle, float dt) {
        if(m_IsPlaying) {
            m_PlayheadTime += dt;
        }

        switch(glfwGetKey(glfwHandle.window(), PLAY_KEY)) {
        case GLFW_PRESS:
            if(!m_IsPlayKeyHeld)
                togglePlayStop();
            m_IsPlayKeyHeld = true;
            break;
        case GLFW_RELEASE:
            m_IsPlayKeyHeld = false;
            break;
        }
        switch(glfwGetKey(glfwHandle.window(), GUI_KEY)) {
        case GLFW_PRESS:
            if(!m_IsGuiKeyHeld)
                m_ShouldGuiBeVisible = !m_ShouldGuiBeVisible;
            m_IsGuiKeyHeld = true;
            break;
        case GLFW_RELEASE:
            m_IsGuiKeyHeld = false;
            break;
        }
    }
};

class Demo {
public:
    Demo(int argc, char** argv);

    int run();
private:
    void update(float dt);
    void renderGUI();
    void renderFrame();
    void renderGeometryForShadowMap();
    void renderGeometry(const glmlv::GLMaterialProgram&);
    void renderGeometryAfterPostFX();
    GLuint getHighestGeometryTextureUnit() const;
    glmlv::Scene& getCurrentScene();
    const glmlv::Scene& getCurrentScene() const;
    float getCurrentSceneDiagonalLength() const;
    glm::vec3 getCurrentSceneCenter() const;
    void changeSceneIDAndConfigure(int sceneID);
    glm::vec3 getShipLeftReactorPosition() const;
    glm::vec3 getShipRightReactorPosition() const;

    // NOTE: Make it static, so that the char pointer's lifetime is unbounded.
    // With the old code, the memory was freed before ImGUI wrote to the ini filename.
    static std::string static_ImGuiIniFilename;

    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "REVOLVE" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. glmlv::GLProgram, GLShader)

    Paths m_Paths;
    int m_PipelineKind;
    ForwardRendering m_ForwardRendering;
    DeferredRendering m_DeferredRendering;
    DirectionalShadowMapping m_DirectionalShadowMapping;
    PostFX m_PostFX;

    glm::vec3 m_ClearColor;
    glmlv::CommonLighting m_Lighting;
    float m_DirLightPhiAngleDegrees; // Angle around Y
    float m_DirLightThetaAngleDegrees; // Angle around X
    glmlv::GLMesh m_ScreenCoverQuad;
    int m_SceneID;

    glmlv::Scene m_BaseBackTopLeft;
    glmlv::Scene m_BaseBase       ;
    glmlv::Scene m_BaseDownLeft   ;
    glmlv::Scene m_BaseFrontLeft  ;
    glmlv::Scene m_BaseLeftBack   ;
    glmlv::Scene m_BaseLeftCenter ;
    glmlv::Scene m_BaseRightBack  ;
    glmlv::Scene m_BaseRight      ;
    glmlv::Scene m_Piste          ;
    glmlv::Scene m_Ship           ;
    glmlv::Scene m_Sun            ;

	glmlv::SceneInstanceData m_EndOfTheWorldInstanceData;
	glmlv::SceneInstanceData m_ShipInstanceData;
    glmlv::SceneInstanceData m_BaseBackTopLeftInstanceData;
    glmlv::SceneInstanceData m_BaseDownLeftInstanceData   ;
    glmlv::SceneInstanceData m_BaseFrontLeftInstanceData  ;
    glmlv::SceneInstanceData m_BaseLeftBackInstanceData   ;
    glmlv::SceneInstanceData m_BaseLeftCenterInstanceData ;
    glmlv::SceneInstanceData m_BaseRightBackInstanceData  ;
    glmlv::SceneInstanceData m_BaseRightInstanceData      ;

    glm::vec3 m_AllShardsAddedDirection;
    glm::vec3 m_BaseBackTopLeftDirection;
    glm::vec3 m_BaseDownLeftDirection   ;
    glm::vec3 m_BaseFrontLeftDirection  ;
    glm::vec3 m_BaseLeftBackDirection   ;
    glm::vec3 m_BaseLeftCenterDirection ;
    glm::vec3 m_BaseRightBackDirection  ;
    glm::vec3 m_BaseRightDirection      ;

    float m_AllShardsVelocityFactor;
    float m_BaseBackTopLeftVelocityFactor;
    float m_BaseDownLeftVelocityFactor   ;
    float m_BaseFrontLeftVelocityFactor  ;
    float m_BaseLeftBackVelocityFactor   ;
    float m_BaseLeftCenterVelocityFactor ;
    float m_BaseRightBackVelocityFactor  ;
    float m_BaseRightVelocityFactor      ;

    float m_ShipReactorStrength;

	glmlv::Scene m_City;
	glmlv::SceneInstanceData m_CityInstanceData;

    glm::vec3 m_SceneCenterForShadowMap;
    float m_SceneRadiusForShadowMap;

    Sprites m_Sprites;
    glmlv::Camera m_Camera;
    float m_CameraMaxSpeed;
    float m_CameraSpeed;
    Skybox m_Skybox;
    ParticlesManager m_ParticlesManager;
    Story m_Story;
};
