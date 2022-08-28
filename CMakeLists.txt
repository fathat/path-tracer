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

# remove this for iterative solver
add_definitions("-DTHREADS")

if (EMSCRIPTEN)

    set(USE_FLAGS "-s USE_SDL=2 -s USE_FREETYPE=1 -O3 -pthread -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1")
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

add_subdirectory(raytracelib)

# add the executable
add_executable(PathTracer 
    main.cpp 
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
target_link_libraries(PathTracer PRIVATE SDL2::SDL2 raytracelib)
else()
target_link_libraries(PathTracer ${SDL2_LIBRARIES} raytracelib)
endif()

if (NOT EMSCRIPTEN)

    # googletest stuff
    include(FetchContent)
    FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(
    test_raytracelib
    test_raytracelib.cpp
    )
    target_link_libraries(
    test_raytracelib
    GTest::gtest_main
    raytracelib
    )

    include(GoogleTest)
    gtest_discover_tests(test_raytracelib)
endif()