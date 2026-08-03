
cmake_minimum_required(VERSION 4.3)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable (VizMa WIN32 "Main.cpp" "VulkanContext.cpp" "Window.cpp" "DEBUG.cpp" "DEBUG.h" "Window.h" "VulkanContext.h" "Shader.h" "Shader.cpp" "GLSLangContext.h" "GLSLangContext.cpp")

target_compile_definitions(VizMa PRIVATE VIZMA_RELEASE)

target_include_directories(VizMa PRIVATE ${VULKAN_INCLUDE_DIR})
target_include_directories(VizMa PRIVATE ${GLSLANG_INCLUDE_DIR})

target_link_libraries(VizMa PRIVATE "${VULKAN_LIB_DIR}/vulkan-1.lib")
target_link_libraries(VizMa PRIVATE 
            "${VULKAN_LIB_DIR}/shaderc_combined.lib"
        )


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VizMa PROPERTY CXX_STANDARD 20)
endif()

