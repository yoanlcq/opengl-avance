#pragma once

#include <cstdlib> // size_t
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ViewController.hpp"
#include "GLFWHandle.hpp"

namespace glmlv {

// An enhanced ViewController that, given perspective parameters, can
// build a perspective projection matrix.
// We might want to add an orthographic projection mode too.
struct Camera: public ViewController {
    float m_FovY = glm::radians(60.f);
    float m_Near = 0.2;
    float m_Far = 5000;
    size_t m_nWindowWidth = 0;
    size_t m_nWindowHeight = 0;

    Camera() = delete;
    Camera(GLFWwindow* window, size_t w, size_t h, float speed = 1):
        ViewController(window, speed),
        m_nWindowWidth(w),
        m_nWindowHeight(h) 
        {}

    float getAspect() const {
        assert(m_nWindowWidth);
        assert(m_nWindowHeight);
        return m_nWindowWidth / float(m_nWindowHeight);
    }
    glm::mat4 getProjMatrix() const {
        assert(m_FovY > 0);
        assert(m_FovY < glm::radians(180.f));
        return glm::perspective(m_FovY, getAspect(), m_Near, m_Far);
    }
};

} // namespace glmlv
