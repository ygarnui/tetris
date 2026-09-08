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

# A dependency-free smoke test: the whole point of this module is that it can be verified
# without pulling in Vulkan/GLFW, so a plain assert-style executable is enough - no gtest/Catch2
# needed for a handful of checks.
add_executable(GameLogicSmokeTest smoketest/main.cpp)
target_link_libraries(GameLogicSmokeTest PRIVATE ${PROJECT_NAME})

if (MSVC)
	target_compile_options(GameLogicSmokeTest PRIVATE /W3)
endif()

add_test(NAME GameLogicSmokeTest COMMAND GameLogicSmokeTest)
