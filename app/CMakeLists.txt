cmake_minimum_required(VERSION 3.22.1)

project(Tetris3D)

include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 3.4
	GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glfw)

set(SRC
	src/asset_manager.h

	src/camera.cpp
	src/camera.h

	src/geometry.cpp
	src/geometry.h

	src/scene.cpp
	src/scene.h

	src/main.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_executable(${PROJECT_NAME} ${SRC})

find_package(Vulkan REQUIRED)

target_include_directories(${PROJECT_NAME}
	PRIVATE SYSTEM ${Vulkan_INCLUDE_DIRS}
	# The structs shared with the shaders live next to them.
	PRIVATE "${CMAKE_SOURCE_DIR}/vulkan_renderer/assets/shaders/globals")

target_link_libraries(${PROJECT_NAME} PRIVATE
	VulkanRenderer
	RenderBase
	Logger
	Utils
	GameLogic
	glfw
)

# The shaders are compiled at runtime, so the application reads them straight out of the
# source tree. Editing a shader and restarting is enough, no rebuild needed.
target_compile_definitions(${PROJECT_NAME} PRIVATE
	TETRIS_SHADER_PATH="${CMAKE_SOURCE_DIR}/vulkan_renderer/assets/shaders/"
	TETRIS_SHADER_CACHE_PATH="${CMAKE_BINARY_DIR}/shader_cache/"
	# Read straight out of the source tree, the same way the shaders are.
	TETRIS_TEXTURE_PATH="${CMAKE_SOURCE_DIR}/vulkan_renderer/assets/textures/"
	GLM_FORCE_DEPTH_ZERO_TO_ONE
	GLM_ENABLE_EXPERIMENTAL
	NOMINMAX
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W3)
	# The Vulkan SDK ships shaderc and spirv-cross without their pdb files.
	target_link_options(${PROJECT_NAME} PRIVATE /ignore:4099)
endif()
