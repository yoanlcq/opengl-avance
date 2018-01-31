#include <AL/alc.h>
#include <AL/al.h>
#include <glm/glm.hpp>
#include "wav.hpp"

namespace glmlv {

static bool checkAlError() {
    auto e = alGetError();
    if(e != AL_NO_ERROR) {
        std::cerr << "OpenAL: An error was generated: " << e << std::endl;
        return false;
    }
    return true;
}
static bool checkAlcError(ALCdevice* dev) {
    auto e = alcGetError(dev);
    if(e != ALC_NO_ERROR) {
        std::cerr << "ALC: An error was generated: " << e << std::endl;
        return false;
    }
    return true;
}

struct ALOrientation {
    glm::vec3 at, up; // NOTE: The order matters!
};
static_assert(sizeof(ALOrientation) == 6*sizeof(float), "");

struct ALHandle {
    ALCdevice* m_Device = nullptr;
    ALCcontext* m_Context = nullptr;

    ~ALHandle() {
        alcDestroyContext(m_Context);
        alcCloseDevice(m_Device);
    }
    ALHandle() {
        m_Device = alcOpenDevice(nullptr);
        if(!m_Device) {
            auto msg = "Unable to init OpenAL default device.";
            std::cerr << msg << std::endl;
            throw std::runtime_error(msg);
        }
        checkAlcError(m_Device);
        std::cout << "Opened OpenAL default device" << std::endl;
        m_Context = alcCreateContext(m_Device, nullptr);
        alcMakeContextCurrent(m_Context);
        checkAlcError(m_Device);

        alGetError(); // clear error code
    }
    ALHandle(const ALHandle& o) = delete;
    ALHandle& operator=(const ALHandle& o) = delete;
    ALHandle(ALHandle&& o) {
        std::swap(m_Device, o.m_Device);
        std::swap(m_Context, o.m_Context);
    }
    ALHandle& operator=(ALHandle&& o) {
        std::swap(m_Device, o.m_Device);
        std::swap(m_Context, o.m_Context);
        return *this;
    }
    void setGain(float gain) const { alListenerf( AL_GAIN, gain); checkAlError(); }
    float getGain() const { float v; alGetListenerf(AL_GAIN, &v); checkAlError(); return v; }
    void setPosition (const glm::vec3& v) const { alListenerfv(AL_POSITION,  &v[0]); checkAlError(); }
    void setVelocity (const glm::vec3& v) const { alListenerfv(AL_VELOCITY,  &v[0]); checkAlError(); }
    void setOrientation(const ALOrientation& v) const { alListenerfv(AL_ORIENTATION, &v.at[0]); checkAlError(); }
    ALOrientation getOrientation() const { ALOrientation v; alGetListenerfv(AL_ORIENTATION, &v.at[0]); checkAlError();  return v; }
    glm::vec3 getPosition () const { glm::vec3 v; alGetListenerfv(AL_POSITION,  &v[0]); checkAlError(); return v; }
    glm::vec3 getVelocity () const { glm::vec3 v; alGetListenerfv(AL_VELOCITY,  &v[0]); checkAlError(); return v; }
};

class ALBuffer {
    ALuint m_ALId = 0;
public:
    ~ALBuffer() {
        alDeleteBuffers(1, &m_ALId);
    }
    ALBuffer() {
        alGenBuffers(1, &m_ALId);
        assert(m_ALId);
        checkAlError();
    }
    ALBuffer(const PcmWav& wav): ALBuffer() {
        loadWav(wav);
    }
    ALBuffer(const ALBuffer& o) = delete;
    ALBuffer& operator=(const ALBuffer& o) = delete;
    ALBuffer(ALBuffer&& o) { std::swap(m_ALId, o.m_ALId); }
    ALBuffer& operator=(ALBuffer&& o) { std::swap(m_ALId, o.m_ALId); return *this; }

    ALuint alId() const { return m_ALId; }
    void loadWav(const PcmWav& wav) const {
        ALenum format = -1;
        /**/ if(wav.channel_count==1 && wav.bits_per_sample== 8) format = AL_FORMAT_MONO8;
        else if(wav.channel_count==1 && wav.bits_per_sample==16) format = AL_FORMAT_MONO16;
        else if(wav.channel_count==2 && wav.bits_per_sample== 8) format = AL_FORMAT_STEREO8;
        else if(wav.channel_count==2 && wav.bits_per_sample==16) format = AL_FORMAT_STEREO16;
        assert(format != -1 && "No suitable OpenAL format found!");

        alBufferData(m_ALId, format, wav.data.data(), wav.data.size(), wav.sample_rate);
        checkAlError();
    }
};

class ALSource {
    ALuint m_ALId = 0;
public:
    ~ALSource() {
        alDeleteSources(1, &m_ALId);
    }
    ALSource() {
        alGenSources(1, &m_ALId);
        assert(m_ALId);
        checkAlError();
    }
    ALSource(const ALBuffer& b): ALSource() {
        setBuffer(b);
    }
    ALSource(const ALSource& o) = delete;
    ALSource& operator=(const ALSource& o) = delete;
    ALSource(ALSource&& o) { std::swap(m_ALId, o.m_ALId); }
    ALSource& operator=(ALSource&& o) { std::swap(m_ALId, o.m_ALId); return *this; }

    ALuint alId() const { return m_ALId; }
    void setBuffer(const ALBuffer& b) const {
        alSourcei(m_ALId, AL_BUFFER, b.alId());
        checkAlError();
    }
    void setLooping(bool loop) const {
        alSourcei(m_ALId, AL_LOOPING, loop);
        checkAlError();
    }
    void play()   const { alSourcePlay  (m_ALId); checkAlError(); }
    void pause()  const { alSourcePause (m_ALId); checkAlError(); }
    void stop()   const { alSourceStop  (m_ALId); checkAlError(); }
    void rewind() const { alSourceRewind(m_ALId); checkAlError(); }
    void setGain(float gain) const { alSourcef(m_ALId, AL_GAIN, gain); checkAlError(); }
    float getGain() const { float v; alGetSourcef(m_ALId, AL_GAIN, &v); checkAlError(); return v; }
    void setPosition (const glm::vec3& v) const { alSourcefv(m_ALId, AL_POSITION,  &v[0]); checkAlError(); }
    void setVelocity (const glm::vec3& v) const { alSourcefv(m_ALId, AL_VELOCITY,  &v[0]); checkAlError(); }
    void setDirection(const glm::vec3& v) const { alSourcefv(m_ALId, AL_DIRECTION, &v[0]); checkAlError(); }
    glm::vec3 getPosition () const { glm::vec3 v; alGetSourcefv(m_ALId, AL_POSITION,  &v[0]); checkAlError(); return v; }
    glm::vec3 getVelocity () const { glm::vec3 v; alGetSourcefv(m_ALId, AL_VELOCITY,  &v[0]); checkAlError(); return v; }
    glm::vec3 getDirection() const { glm::vec3 v; alGetSourcefv(m_ALId, AL_DIRECTION, &v[0]); checkAlError(); return v; }
    enum class State { Playing, Paused, Stopped };
    State getState() const {
        ALint i;
        alGetSourcei(m_ALId, AL_SOURCE_STATE, &i);
        checkAlError();
        switch(i) {
        case AL_PLAYING: return State::Playing;
        case AL_PAUSED : return State::Paused;
        case AL_STOPPED: return State::Stopped;
        }
        assert(false && "alGetSourcei returned an invalid state!");
    }
    float  getPlaybackPositionInSeconds() const { ALfloat v; alGetSourcef(m_ALId, AL_SEC_OFFSET, &v);  checkAlError(); return v; }
    size_t getPlaybackPositionInSamples() const { ALint i; alGetSourcei(m_ALId, AL_SAMPLE_OFFSET, &i); checkAlError(); return i; }
    size_t getPlaybackPositionInBytes()   const { ALint i; alGetSourcei(m_ALId, AL_BYTE_OFFSET, &i);   checkAlError(); return i; }
};

} // namespace glmlv
