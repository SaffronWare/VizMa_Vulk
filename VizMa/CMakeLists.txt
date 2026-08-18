
cmake_minimum_required(VERSION 4.3)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable (VizMa WIN32 "Main.cpp" 
"VulkanContext.cpp" 
"Window.cpp"
"DEBUG.cpp" "DEBUG.h" "Window.h" "VulkanContext.h"   "GLSLangContext.h" "GLSLangContext.cpp" "FileUtils.h" "FileUtils.cpp" "Camera.h" "Camera.cpp")

# set to VIZMA_DEBUG for DEBUG & VIZMA_RELEASE for RELEASE (this does nothing for now lol)
target_compile_definitions(VizMa PRIVATE VIZMA_DEBUG)

target_include_directories(VizMa PRIVATE ${VULKAN_INCLUDE_DIR})
target_include_directories(VizMa PRIVATE ${GLSLANG_INCLUDE_DIR})
target_include_directories(VizMa PRIVATE ${ARKMAT_INCLUDE_DIR})
target_link_libraries(VizMa PRIVATE "${VULKAN_LIB_DIR}/vulkan-1.lib")
target_link_libraries(VizMa PRIVATE 
            "${VULKAN_LIB_DIR}/shaderc_combined.lib"
            "${VULKAN_LIB_DIR}/glslang-default-resource-limits.lib"
        )


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VizMa PROPERTY CXX_STANDARD 20)
endif()

