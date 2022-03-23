

cmake_minimum_required(VERSION 3.22 FATAL_ERROR)


project(Patch)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(lib/glfw)

add_subdirectory(lib/GLAD)

#include_directories(lib/GLAD/include)
#include_directories(lib/glfw/include)

add_executable(Patcher src/main.cpp)

target_link_libraries(Patcher glfw)
target_link_libraries(Patcher glad)

