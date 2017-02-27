#pragma once

#include "GLObject.hpp"

namespace voxskel {

class GLVertexArray: GLVertexArrayObject {
public:
    using GLVertexArrayObject::glId;

    GLVertexArray() = default;

    GLVertexArray(GLVertexArray&& rvalue) : GLVertexArrayObject(std::move(rvalue)) {
    }

    GLVertexArray& operator =(GLVertexArray&& rvalue) {
        GLVertexArrayObject::operator=(std::move(rvalue));
        return *this;
    }

    void bind() const {
        glBindVertexArray(glId());
    }

    void enableVertexAttrib(GLuint index) {
        glEnableVertexArrayAttrib(glId(), index);
    }

    void disableVertexAttrib(GLuint index) {
        glDisableVertexArrayAttrib(glId(), index);
    }
};

}
