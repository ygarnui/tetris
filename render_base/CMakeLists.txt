cmake_minimum_required(VERSION 3.22.1)

project(RenderBase)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -MP")

set(SRC
	include/buffer_description.h
	include/cmd_calls.h
	include/data_shader_module_reflection.h
	include/directions.h
	include/draw_priority.h
	include/drawcall.h
	include/format.h
	include/generator_id.h
	include/globals.h
	include/graphics_id.h
	include/interface_image.h
	include/interface_manager_asset.h
	include/interface_manager_asset_render.h
	include/manager_drawcall.h
	include/manager_frame_buffer.h
	include/manager_command_buffer.h
	include/manager_uniform_buffer.h
	include/manager_buffer.h
	include/manager_textures.h
	include/manager_pipeline.h
	include/manager_shader_program.h
	include/manager_render_pass.h
	include/render_base.h
	include/render_id.h
	include/renderpass_create_info.h
	include/viewport.h
	include/sampler_create_info.h	
	include/manager_assets.h
	src/render_base.cpp
	src/format.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} ${SRC})
add_library(RENDER_BASE ALIAS ${PROJECT_NAME})

#---find Vulkan, used for GLM---
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
	message(SEND_ERROR "Failed to find Vulkan")
	return()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "include" PRIVATE SYSTEM ${Vulkan_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC
	${Vulkan_LIBRARY}
	Logger
)

#supress warnings
if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /WX)    
endif()
