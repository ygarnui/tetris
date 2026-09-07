cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 20)

project(Logger)

set(SRC
	logger.cpp
	logger.h

	logger_instance.cpp
	logger_instance.h
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} SHARED ${SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC
	Utils
)

#supress warnings
if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /WX /W3)    
endif()
