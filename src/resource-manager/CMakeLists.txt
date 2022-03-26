
cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(resource_manager)

set(CXX_STANDARD 17)

add_library(resource_manager STATIC src/shader_handler.h src/shader_handler.cpp src/murmurhash.h src/murmurhash.cpp
                                    ${CMAKE_SOURCE_DIR}/lib/log.h ${CMAKE_SOURCE_DIR}/lib/log.cpp)

target_link_libraries(resource_manager glad)

include_directories(${CMAKE_SOURCE_DIR}/lib)
target_include_directories(resource_manager PUBLIC src)

