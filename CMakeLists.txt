cmake_minimum_required(VERSION 3.10)

set(SDL2_DIR "${CMAKE_CURRENT_LIST_DIR}/3rdParty/SDL2-2.0.22")

# set the project name
project(PathTracer VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB PathTracer_HEADERS *.h)
file(GLOB PathTracer_SOURCE *.cpp)

source_group("Headers" FILES ${PathTracer_HEADERS})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

include_directories("${CMAKE_CURRENT_LIST_DIR}/3rdParty")

# add the executable
add_executable(PathTracer 
    main.cpp 
    color.h 
    ray.h 
    types.h
    image_buffer.h
    image_buffer.cpp)
target_link_libraries(PathTracer ${SDL2_LIBRARIES})