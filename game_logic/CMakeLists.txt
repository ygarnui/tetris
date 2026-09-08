cmake_minimum_required(VERSION 3.22.1)

project(GameLogic)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -MP")

set(SRC
	include/types.h
	include/tetromino.h
	include/board.h
	include/randomizer.h
	include/game.h

	src/tetromino.cpp
	src/board.cpp
	src/randomizer.cpp
	src/game.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC})

add_library(${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME} PUBLIC "include")

if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /WX /W3)
endif()
