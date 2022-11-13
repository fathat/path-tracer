find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
find_package(Vulkan REQUIRED)



add_executable(rtvk
    vk_types.h
    VkBootstrap.h
    VkBootstrap.cpp
    vkengine.h
    vkengine.cpp
    vkmain.cpp
)

set_property(TARGET rtvk PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:rtvk>")
target_include_directories(rtvk PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${RTLIB_DIR}")
target_link_libraries(rtvk Vulkan::Vulkan ${SDL2_LIBRARIES} raytracelib)