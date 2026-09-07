cmake_minimum_required(VERSION 3.22.1)

project(RayTracedTetris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# all binaries in one place so the Logger dll sits next to the executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if (MSVC)
    add_compile_options(/MP)
endif()

# order matters: vulkan_renderer reads ${RenderBase_SOURCE_DIR} at configure time
add_subdirectory(utils/utils)
add_subdirectory(utils/logger)
add_subdirectory(render_base)
add_subdirectory(image_loader)
add_subdirectory(vulkan_renderer)
add_subdirectory(app)
