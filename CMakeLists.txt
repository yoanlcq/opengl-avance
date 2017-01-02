cmake_minimum_required(VERSION 2.8)

project(opengl)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(IMGUI_DIR imgui-1.49)
set(GLFW_DIR glfw-3.2.1)
set(GLM_DIR glm-0.9.8.3)
set(GLAD_DIR glad)
set(JSON_DIR json-2.0.9)

add_subdirectory(third-party/${GLFW_DIR})

find_package(OpenGL REQUIRED)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(c2ba-glsl-shaders)
include(c2ba-copy-dll-post-build)

include_directories(
    glmlv
    PUBLIC 
    ${OPENGL_INCLUDE_DIRS}
    third-party/${GLFW_DIR}/include
    third-party/${GLM_DIR}
    third-party/${GLAD_DIR}/include
    third-party/${JSON_DIR}
    third-party/${IMGUI_DIR}
    lib/include
)

file(
    GLOB_RECURSE
    GLMLV_LIB_FILES
    lib/*.cpp lib/*.hpp
)

add_library(
    glmlv
    ${GLMLV_LIB_FILES}
)

set(
    IMGUI_SRC_FILES
    third-party/${IMGUI_DIR}/imgui.cpp
    third-party/${IMGUI_DIR}/imgui_demo.cpp
    third-party/${IMGUI_DIR}/imgui_draw.cpp
)

file(
    GLOB_RECURSE
    SRC_FILES
    apps/*.cpp apps/*.hpp apps/*.glsl
)

add_executable(
    template
    ${SRC_FILES}
    third-party/${GLAD_DIR}/src/glad.c
    ${IMGUI_SRC_FILES}
)

target_link_libraries(
    template
    ${OPENGL_LIBRARIES}
    glfw
    glmlv
)

source_group ("glsl" REGULAR_EXPRESSION "*.glsl")
source_group ("third-party" REGULAR_EXPRESSION "third-party/*.*")

install(
    TARGETS template
    DESTINATION bin
)