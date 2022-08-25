cmake_minimum_required(VERSION 3.10)

if (WIN32)
set(SDL2_DIR "${CMAKE_CURRENT_LIST_DIR}/3rdParty/SDL2-2.0.22")
endif()
set(IMGUI_DIR "${CMAKE_CURRENT_LIST_DIR}/3rdParty/imgui")

# set the project name
project(PathTracer VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (EMSCRIPTEN)
    set(USE_FLAGS "-s USE_SDL=2 -s USE_FREETYPE=1 -O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
endif()

file(GLOB PathTracer_HEADERS *.h)
file(GLOB PathTracer_SOURCE *.cpp)
file(GLOB IMGUI_HEADERS 3rdPart/imgui/*.h)
file(GLOB IMGUI_SOURCE 3rdPart/imgui/*.cpp)

source_group("Headers" FILES ${PathTracer_HEADERS})
source_group("IMGUI Headers" FILES ${IMGUI_HEADERS})
source_group("IMGUI Source" FILES ${IMGUI_SOURCE})

if (APPLE)
find_package(SDL2 REQUIRED)
else()
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
endif()

include_directories(${IMGUI_DIR})

include_directories("${CMAKE_CURRENT_LIST_DIR}/3rdParty")

# add the executable
add_executable(PathTracer 
    main.cpp 
    color.h 
    ray.h 
    types.h
    types.cpp
    image_buffer.h
    image_buffer.cpp
    debug_utils.h
    debug_utils.cpp
    hittable.h
    hittable.cpp
    sphere.h
    sphere.cpp
    hittable_list.h
    hittable_list.cpp
    camera.h
    camera.cpp
    material.h
    material.cpp
    texture.h
    texture.cpp
    3rdParty/imgui/imconfig.h
    3rdParty/imgui/imgui_draw.cpp
    3rdParty/imgui/imgui_impl_sdl.cpp
    3rdParty/imgui/imgui_impl_sdl.h
    3rdParty/imgui/imgui_impl_sdlrenderer.cpp
    3rdParty/imgui/imgui_impl_sdlrenderer.h
    3rdParty/imgui/imgui_internal.h
    3rdParty/imgui/imgui_tables.cpp
    3rdParty/imgui/imgui_widgets.cpp
    3rdParty/imgui/imgui.cpp
    3rdParty/imgui/imgui.h
    3rdParty/imgui/imstb_rectpack.h
    3rdParty/imgui/imstb_textedit.h
    3rdParty/imgui/imstb_truetype.h
    )

if (APPLE)
target_link_libraries(PathTracer PRIVATE SDL2::SDL2)
else()
target_link_libraries(PathTracer ${SDL2_LIBRARIES})
endif()
