cmake_minimum_required(VERSION 3.22.1)

project(ImageLoader)

set(SRC
	include/image_loader.h
	include/interface_image_object.h
	include/stb_image_write.h
	
	image_loader.cpp
	
	stb_image.h
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} ${SRC})
add_library(IMAGELOADER ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC "include")

target_link_libraries(${PROJECT_NAME} PUBLIC
	Logger
	RenderBase
)
