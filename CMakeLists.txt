cmake_minimum_required(VERSION 2.8)

project(gl-utilities)

set(glext_h "${CMAKE_SOURCE_DIR}/include/GL/glext.h")
set(glcorearb_h "${CMAKE_SOURCE_DIR}/include/GL/glcorearb.h")

if(NOT EXISTS ${glext_h})
    message(STATUS "Downloading glext.h")
    file(DOWNLOAD https://www.opengl.org/registry/api/GL/glext.h ${glext_h})
endif()

if(NOT EXISTS ${glcorearb_h})
    message(STATUS "Downloading glcorearb.h")
    file(DOWNLOAD https://www.opengl.org/registry/api/GL/glcorearb.h ${glcorearb_h})
endif()

set(GL_HEADERS
    include/GL/glext.h
    include/GL/glcorearb.h
    include/GL/glxext.h
    include/GL/wglext.h
    )

add_executable(gl-utilities-extensions
    src/program.cpp
    src/reading.cpp
    src/writing.cpp
    src/glextloader.h
    )

target_include_directories(gl-utilities-extensions
    PRIVATE ${CMAKE_SOURCE_DIR}
    )

target_compile_features(gl-utilities-extensions
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    )

add_custom_command(TARGET gl-utilities-extensions POST_BUILD
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/gl-utilities-extensions ${CMAKE_CURRENT_SOURCE_DIR}
    )

if(WIN32)
    find_package(OpenGL REQUIRED)
endif()

set(SRC_Headers
    include/gl-utilities-loaders.h
    include/gl-utilities-shaders.h
    include/gl-utilities-textures.h
    include/gl-utilities-vertexbuffers.h
    )

add_library(gl-utilities
    src/gl-utilities.cpp
    ${SRC_Headers}
    )

target_include_directories(gl-utilities
    PRIVATE include
    )

target_link_libraries(gl-utilities
    ${OPENGL_LIBRARIES}
    )

target_compile_features(gl-utilities
    PRIVATE cxx_nullptr
    )
