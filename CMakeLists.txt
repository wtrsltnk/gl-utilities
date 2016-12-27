cmake_minimum_required(VERSION 2.8)

project(shadertest)

include(win-cpp-deps.cmake/win-cpp-deps.cmake)

install_dep("https://bitbucket.org/wincppdeps/glfw.git")
install_dep("https://bitbucket.org/wincppdeps/glm.git")
install_dep("https://bitbucket.org/wincppdeps/glextloader.git")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ENABLE_CXX11} -std=c++11")

if(WIN32)
    find_package(OpenGL REQUIRED)
endif()

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIR})

find_package(GLFW3 REQUIRED)
include_directories(${GLFW3_INCLUDE_DIR})

set(SRC_Headers
    gl.utilities.h
    )

set(SRC_Common
    examples/glfw-setup.cpp
    examples/glfw-setup.h
    examples/stb_image.h
    examples/glextl.cpp
    )

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/examples
    )

add_library(common ${SRC_Common})



add_executable(VertexAndColorExample
    ${SRC_Headers}
    examples/01-VertexAndColorExample/program.cpp
    )

target_link_libraries(VertexAndColorExample
    common
    ${GLFW3_LIBRARY}
    ${OPENGL_LIBRARIES}
    )



add_executable(VertexNormalAndTexcoordExample
    ${SRC_Headers}
    examples/02-VertexNormalAndTexcoordExample/program.cpp
    )

target_link_libraries(VertexNormalAndTexcoordExample
    common
    ${GLFW3_LIBRARY}
    ${OPENGL_LIBRARIES}
    )



add_executable(VertexNormalTexcoordAndColorExample
    ${SRC_Headers}
    examples/03-VertexNormalTexcoordAndColorExample/program.cpp
    )

target_link_libraries(VertexNormalTexcoordAndColorExample
    common
    ${GLFW3_LIBRARY}
    ${OPENGL_LIBRARIES}
    )
