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
#include <glm/gtc/random.hpp>

void handleFboStatus(GLenum status);

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
private:

    GLuint m_Vao = 0, m_PositionBo = 0, m_VelocityBo = 0;
    const size_t m_MaxParticleCount;
    size_t m_ParticleCount;
    const Shape m_Shape;
    float m_SpawnRadius;

    struct HostData {
        std::vector<glm::vec4> pos, vel;

        HostData(size_t count, Shape shape, float radius):
            pos(count), vel(count)
        {
            switch(shape) {
            case Shape::Sphere:
                for(size_t i=0 ; i<count ; ++i) {
                    pos[i] = glm::vec4(glm::sphericalRand(radius), 1);
                    vel[i] = glm::vec4(glm::sphericalRand(radius) / 10.f, 0);
                }
                break;
            case Shape::Disk:
                for(size_t i=0 ; i<count ; ++i) {
                    const auto ax = glm::radians(glm::linearRand(-60.f, 60.f));
                    const auto ay = glm::radians(glm::linearRand(-60.f, 60.f));
                    const auto rx = glm::rotate(glm::mat4(1), ax, glm::vec3(1,0,0));
                    const auto ry = glm::rotate(glm::mat4(1), ay, glm::vec3(0,1,0));
                    vel[i] = rx * ry * glm::vec4(0,0,1,0);
                    pos[i] = glm::vec4(glm::diskRand(radius), 0, 1);
                }
                break;
            }
        }
    };

public:

    struct InstanceData {
        // Compute shader - update
        float velMultiplier   = 1.f;
        // Vertex-Fragment shader - render
        glm::vec3 origin      = glm::vec3(0.f);
        float pointSize       = 8.f;
        float zScale          = 1000.f;
        float zInfluence      = 1.f;
        glm::vec4 innerColor  = glm::vec4(1.f, 0.9f, 0.3f, 1.0f);
        glm::vec4 outerColor  = glm::vec4(1.f, 0.2f, 0.0f, 0.0f);

        glm::mat4 getModelMatrix() const {
            return glm::translate(glm::mat4(1.f), origin);
        }
    };

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

        addParticles(count);

        glBindVertexArray(m_Vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_PositionBo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);
    }

    size_t getParticleCount() const { return m_ParticleCount; }
    size_t getMaxParticleCount() const { return m_MaxParticleCount; }

    void addParticles(size_t count) {
        if(m_ParticleCount >= m_MaxParticleCount)
            return;

        count = std::min(count, m_MaxParticleCount - m_ParticleCount);
        assert(m_ParticleCount + count <= m_MaxParticleCount);
        const HostData d(count, m_Shape, m_SpawnRadius);

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
            camera.getProjMatrix() * camera.getViewMatrix() * i.getModelMatrix()
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

public:
    PostFX_ComputePassProgram(const glmlv::fs::path& cs):
        GLProgram(glmlv::compileProgram({ cs.string() })),
        m_UniformInputImageLocation           (getUniformLocation("uInputImage")),
        m_UniformOutputImageLocation          (getUniformLocation("uOutputImage")),
        m_UniformGammaExponentLocation        (getUniformLocation("uGammaExponent")),
        m_UniformFinalTouchMulLocation        (getUniformLocation("uFinalTouchMul")),
        m_UniformFinalTouchAddLocation        (getUniformLocation("uFinalTouchAdd"))
        {}
    void setUniformInputImage(GLint i)               const { glUniform1i(m_UniformInputImageLocation, i); }
    void setUniformOutputImage(GLint i)              const { glUniform1i(m_UniformOutputImageLocation, i); }
    void setUniformGammaExponent(GLfloat f)          const { glUniform1f(m_UniformGammaExponentLocation, f); }
    void setUniformFinalTouchMul(const glm::vec3 &v) const { glUniform3fv(m_UniformFinalTouchMulLocation, 1, &v[0]); }
    void setUniformFinalTouchAdd(const glm::vec3 &v) const { glUniform3fv(m_UniformFinalTouchAddLocation, 1, &v[0]); }
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

struct ParticlesManager {
    GLParticlesSimulationProgram m_SimulationProgram;
    GLParticlesRenderingProgram m_RenderingProgram;
    Particles m_ToastParticles;
    glm::vec3 m_ToastParticlesOrigin;
    Particles::InstanceData m_ToastParticlesInstanceData;

    ParticlesManager(const Paths& paths): 
        m_SimulationProgram(paths.m_AppShaders / "particles.cs.glsl"),
        m_RenderingProgram(
            paths.m_AppShaders / "particles.vs.glsl",
            paths.m_AppShaders / "particles.fs.glsl"
        ),
        m_ToastParticles(1<<17, Particles::Shape::Disk, 321.f)
    {
        glEnable(GL_PROGRAM_POINT_SIZE);
    }
    void render(const glmlv::Camera& cam) const {
        m_RenderingProgram.use();
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        m_ToastParticles.render(m_RenderingProgram, cam, m_ToastParticlesInstanceData);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
    void update(float dt) {
        m_SimulationProgram.use();
        m_ToastParticles.update(m_SimulationProgram, dt, m_ToastParticlesInstanceData);
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
    // Add new skybox cubemaps here
    enum Sky {
        SkySimpleColorTest = 0,
        SkySpaceKurt,
        SkySpaceUlukaiCorona,
        SkySpaceUlukaiRedEclipse,
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
        m_CurrentSky(SkySpaceUlukaiCorona)
    {
        glmlv::CubeMapFaceImages faces;

        faces.px = glmlv::Image2DRGBA(128, 128, 255, 000, 000, 255);
        faces.py = glmlv::Image2DRGBA(128, 128, 000, 255, 000, 255);
        faces.pz = glmlv::Image2DRGBA(128, 128, 000, 000, 255, 255);
        faces.nx = glmlv::Image2DRGBA(128, 128, 128, 000, 000, 255);
        faces.ny = glmlv::Image2DRGBA(128, 128, 000, 128, 000, 255);
        faces.nz = glmlv::Image2DRGBA(128, 128, 000, 000, 128, 255);
        m_Skies[SkySimpleColorTest].uploadImages(faces);

        faces.px = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "kurt" / "space_rt.png");
        faces.py = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "kurt" / "space_up.png");
        faces.pz = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "kurt" / "space_bk.png");
        faces.nx = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "kurt" / "space_lf.png");
        faces.ny = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "kurt" / "space_dn.png");
        faces.nz = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "kurt" / "space_ft.png");
        m_Skies[SkySpaceKurt].uploadImages(faces);

        faces.px = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_rt.png");
        faces.py = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_up.png");
        faces.pz = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_bk.png");
        faces.nx = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_lf.png");
        faces.ny = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_dn.png");
        faces.nz = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "corona_ft.png");
        m_Skies[SkySpaceUlukaiCorona].uploadImages(faces);

        faces.px = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_rt.png");
        faces.py = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_up.png");
        faces.pz = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_bk.png");
        faces.nx = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_lf.png");
        faces.ny = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_dn.png");
        faces.nz = glmlv::readImage(paths.m_AppAssets / "skyboxes" / "ulukai" / "redeclipse_ft.png");
        m_Skies[SkySpaceUlukaiRedEclipse].uploadImages(faces);
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
        m_CubeMesh.render();
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

    PostFX_ComputePass(const Paths& paths, GLsizei w, GLsizei h):
        m_Program(paths.m_AppShaders / "postFX.cs.glsl"),
        m_InputDepthTexture(GL_DEPTH_COMPONENT32F, w, h),
        m_Input(w, h),
        m_Output(w, h),
        m_IsEnabled(false),
        m_Gamma(2.2f),
        m_FinalTouchMul(1.0f),
        m_FinalTouchAdd(0.0f)
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

class Demo {
public:
    Demo(int argc, char** argv);

    int run();
private:
    void renderGUI();
    void renderFrame();
    void renderGeometry();
    void renderGeometry(const glmlv::GLMaterialProgram&);
    GLuint getHighestGeometryTextureUnit() const;

    // NOTE: Make it static, so that the char pointer's lifetime is unbounded.
    // With the old code, the memory was freed before ImGUI wrote to the ini filename.
    static std::string static_ImGuiIniFilename;

    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Creating Dimensions" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. glmlv::GLProgram, GLShader)

    Paths m_Paths;
    static const int PIPELINE_FORWARD = 1;
    static const int PIPELINE_DEFERRED = 2;
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
    glmlv::Scene m_Sponza;
    glmlv::SceneInstanceData m_SponzaInstanceData;
    glmlv::Camera m_Camera;
    const float m_CameraMaxSpeed;
    float m_CameraSpeed;
    Skybox m_Skybox;
    ParticlesManager m_ParticlesManager;
    bool m_IsDemoPlaying;
};
