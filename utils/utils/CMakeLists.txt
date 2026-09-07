cmake_minimum_required(VERSION 3.22.1)

project(Utils)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -MP")

set(SRC
	include/collection_utils.h
	include/guard_next_id.h
	include/vector.h
	include/vector_utils.h
	include/queue_command.h
	src/collection_utils.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} ${SRC})
add_library(UTILSLIB ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC "include")

#supress warnings
if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /WX /W3)    
endif()
