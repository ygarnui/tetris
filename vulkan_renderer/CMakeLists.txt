cmake_minimum_required(VERSION 3.22.1)

project(VulkanRenderer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -MP")

set(SOURCE_FILE "${RenderBase_SOURCE_DIR}/include/globals.h")
set(TARGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/globals")
file(COPY ${SOURCE_FILE} DESTINATION ${TARGET_DIR})

set(SRC

	src/buffers/creator_buffer.cpp
	src/buffers/creator_buffer.h
	
	src/buffers/creator_descriptor_pool.cpp
	src/buffers/creator_descriptor_pool.h
	
	src/buffers/creator_descriptor_set.cpp
	src/buffers/creator_descriptor_set.h
	
	src/buffers/creator_descriptor_set_layout.cpp
	src/buffers/creator_descriptor_set_layout.h
	
	src/buffers/creator_index_buffer.cpp
	src/buffers/creator_index_buffer.h
	
	src/buffers/creator_uniform_buffer.cpp
	src/buffers/creator_uniform_buffer.h
	
	src/buffers/creator_vertex_buffer.cpp
	src/buffers/creator_vertex_buffer.h
	
	src/buffers/vulkan_manager_buffer.cpp
	src/buffers/vulkan_manager_buffer.h
	
	src/buffers/vulkan_manager_uniform_buffer.cpp
	src/buffers/vulkan_manager_uniform_buffer.h
	
	src/pipeline/command_buffer.cpp
	src/pipeline/command_buffer.h
	
	src/pipeline/commands.cpp
	src/pipeline/commands.h
	
	src/pipeline/creator_command_buffer.cpp
	src/pipeline/creator_command_buffer.h
	
	src/pipeline/creator_compute_pipeline.cpp
	src/pipeline/creator_compute_pipeline.h

	src/pipeline/creator_graphics_pipeline.cpp
	src/pipeline/creator_graphics_pipeline.h
	
	src/pipeline/creator_pipeline_layout.cpp
	src/pipeline/creator_pipeline_layout.h
	
	src/pipeline/vulkan_manager_command_buffer.cpp
	src/pipeline/vulkan_manager_command_buffer.h

	src/pipeline/vulkan_manager_pipeline.cpp
	src/pipeline/vulkan_manager_pipeline.h
	
	src/pipeline/vulkan_drawcall.cpp
	src/pipeline/vulkan_drawcall.h

	src/pipeline/vulkan_manager_drawcall.cpp
	src/pipeline/vulkan_manager_drawcall.h
	
	src/raytracing/creator_acceleration_structure.cpp
	src/raytracing/creator_acceleration_structure.h

	src/raytracing/creator_ray_tracing_pipeline.cpp
	src/raytracing/creator_ray_tracing_pipeline.h

	src/raytracing/extension_functions.cpp
	src/raytracing/extension_functions.h

	src/raytracing/manager_ray_tracing.cpp
	src/raytracing/manager_ray_tracing.h

	src/shader/compiler_shader_module.cpp
	src/shader/compiler_shader_module.h

	src/shader/creator_shader_module.cpp
	src/shader/creator_shader_module.h
	
	src/shader/manager_shader_module.cpp
	src/shader/manager_shader_module.h
	
	src/shader/vulkan_manager_shader_program.cpp
	src/shader/vulkan_manager_shader_program.h
	
	src/shader/reader_shader.cpp
	src/shader/reader_shader.h

	src/shader/shader_includer.cpp
	src/shader/shader_includer.h
	
	src/textures/creator_image_buffer.cpp
	src/textures/creator_image_buffer.h
	
	src/textures/creator_texture_sampler.h
	src/textures/creator_texture_sampler.cpp
	
	src/textures/manager_sampler.cpp
	src/textures/manager_sampler.h

	src/textures/vulkan_manager_textures.cpp
	src/textures/vulkan_manager_textures.h
	
	src/utils/guard_time.h
	src/utils/guard_time.cpp

	src/utils/manager_time.h
	src/utils/manager_time.cpp

	src/vma_implementation.cpp

	src/converter_description.cpp
	src/converter_description.h

	src/creator_command_pool.cpp
	src/creator_command_pool.h
	
	src/creator_debug_utils_messenger.cpp
	src/creator_debug_utils_messenger.h
	
	src/creator_fence.cpp
	src/creator_fence.h

	src/creator_frame_buffer.cpp
	src/creator_frame_buffer.h
	
	src/creator_image_views.cpp
	src/creator_image_views.h
	
	src/creator_images.cpp
	src/creator_images.h

	src/creator_instance.cpp
	src/creator_instance.h

	src/creator_logical_device.cpp
	src/creator_logical_device.h

	src/creator_physical_device.cpp
	src/creator_physical_device.h
	
	src/creator_queue_descriptor.cpp
	src/creator_queue_descriptor.h
	
	src/creator_queue_families.cpp
	src/creator_queue_families.h
	
	src/creator_render_pass.cpp
	src/creator_render_pass.h
	
	src/creator_semaphore.cpp
	src/creator_semaphore.h

	src/creator_surface_khr.cpp
	src/creator_surface_khr.h

	src/creator_swapchain.cpp
	src/creator_swapchain.h

	src/device_property.h
	
	src/filter_suitable_devices.cpp
	src/filter_suitable_devices.h
	
	src/vulkan_manager_assets.h
	
	src/manager_base.h
	
	src/vulkan_manager_frame_buffer.cpp
	src/vulkan_manager_frame_buffer.h

	src/manager_device.cpp
	src/manager_device.h
	
	src/vulkan_manager_render_pass.cpp
	src/vulkan_manager_render_pass.h
	
	src/manager_surface.cpp
	src/manager_surface.h
	
	src/manager_swapchain.cpp
	src/manager_swapchain.h
	
	src/creator_swapchain_support_details.cpp
	src/creator_swapchain_support_details.h

	src/manager_window.cpp
	src/manager_window.h
	
	src/required_device_parameters.h

	src/struct_data.h
	src/struct_base_data.h
	
	src/selector_swapchain_settings.cpp
	src/selector_swapchain_settings.h
	
	src/validation_layer.cpp
	src/validation_layer.h
	
	src/vulkan_render_base.cpp
	include/vulkan_render_base.h


	assets/shaders/globals/camera.h
	assets/shaders/globals/convert_uint_to_vec4.h
	assets/shaders/globals/globals.h
	assets/shaders/globals/node.h

	












	
	
	
	
	







	
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} STATIC ${SRC})
add_library(VKRENDERER ALIAS ${PROJECT_NAME})

#---find Vulkan---
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
	message(SEND_ERROR "Failed to find Vulkan")
	return()
endif()

#---find shaderc compiler release---
find_library(shaderc_combined_r_LIBRARY NAMES shaderc_combined HINTS $ENV{VK_SDK_PATH}/Lib REQUIRED)
if(NOT shaderc_combined_r_LIBRARY)
	message(SEND_ERROR "Failed to find shaderc_combined library release")
	return()
endif()

#---find shaderc compiler debug---
find_library(shaderc_combined_d_LIBRARY NAMES shaderc_combinedd HINTS $ENV{VK_SDK_PATH}/Lib REQUIRED)
if(NOT shaderc_combined_d_LIBRARY)
	message(SEND_ERROR "Failed to find shaderc_combinedd library debug")
	return()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE debug ${shaderc_combined_d_LIBRARY} optimized ${shaderc_combined_r_LIBRARY})

#---find spirv cross core release---
find_library(spirv_cross_r_LIBRARY NAMES spirv-cross-core HINTS $ENV{VK_SDK_PATH}/Lib REQUIRED)
if(NOT spirv_cross_r_LIBRARY)
	message(SEND_ERROR "Failed to find spirv-cross-core library release")
	return()
endif()

#---find spirv cross core debug---
find_library(spirv_cross_d_LIBRARY NAMES spirv-cross-cored HINTS $ENV{VK_SDK_PATH}/Lib REQUIRED)
if(NOT spirv_cross_d_LIBRARY)
	message(SEND_ERROR "Failed to find spirv-cross-cored library debug")
	return()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE debug ${spirv_cross_d_LIBRARY} optimized ${spirv_cross_r_LIBRARY})

#---find spirv cross glsl release---
find_library(spirv_cross_hlsl_r_LIBRARY NAMES spirv-cross-glsl HINTS $ENV{VK_SDK_PATH}/Lib REQUIRED)
if(NOT spirv_cross_hlsl_r_LIBRARY)
	message(SEND_ERROR "Failed to find spirv-cross-glsl library release")
	return()
endif()

#---find spirv cross glsl debug---
find_library(spirv_cross_hlsl_d_LIBRARY NAMES spirv-cross-glsld HINTS $ENV{VK_SDK_PATH}/Lib REQUIRED)
if(NOT spirv_cross_hlsl_d_LIBRARY)
	message(SEND_ERROR "Failed to find spirv-cross-glsld library debug")
	return()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE debug ${spirv_cross_hlsl_d_LIBRARY} optimized ${spirv_cross_hlsl_r_LIBRARY})

#---find Shaderc reflect release---
find_library(spirv_cross_reflect_r_LIBRARY NAMES spirv-cross-reflect HINTS $ENV{VK_SDK_PATH}/Lib REQUIRED)
if(NOT spirv_cross_reflect_r_LIBRARY)
	message(SEND_ERROR "Failed to find spirv-cross-reflect library release")
	return()
endif()

#---find spirv cross reflect debug---
find_library(spirv_cross_reflect_d_LIBRARY NAMES spirv-cross-reflectd HINTS $ENV{VK_SDK_PATH}/Lib REQUIRED)
if(NOT spirv_cross_reflect_d_LIBRARY)
	message(SEND_ERROR "Failed to find spirv-cross-reflectd library debug")
	return()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE debug ${spirv_cross_reflect_d_LIBRARY} optimized ${spirv_cross_reflect_r_LIBRARY})

# "src" is public because the application drives the ray tracing managers directly:
# the public facade is built around the raster pipeline and has no ray tracing entry points.
target_include_directories(${PROJECT_NAME} PUBLIC "include" "src" PRIVATE SYSTEM ${Vulkan_INCLUDE_DIRS} )

target_link_libraries(${PROJECT_NAME} PUBLIC 
	${Vulkan_LIBRARY}
	ImageLoader
	Utils
	Logger
	RenderBase
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

#supress warnings
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
	target_compile_options(${PROJECT_NAME} PRIVATE /WX /W3)
	# VMA_IMPLEMENTATION expands into third-party code this project doesn't control - don't hold
	# it to /WX.
	set_source_files_properties(src/vma_implementation.cpp PROPERTIES COMPILE_OPTIONS "/W0")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	target_compile_options(${PROJECT_NAME} PRIVATE -W3) 
endif()

target_link_options(${PROJECT_NAME} PRIVATE /ignore:4099)
