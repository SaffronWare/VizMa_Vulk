cmake_minimum_required(VERSION 4.3)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VULKAN_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/vulkan/1.4.357.0/include")
set(VULKAN_LIB_DIR "${CMAKE_SOURCE_DIR}/dependencies/vulkan/1.4.357.0/Lib")

set(GLSLANG_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/vulkan/1.4.357.0/Include/glslang/Public")


add_subdirectory(VizMa)

 



