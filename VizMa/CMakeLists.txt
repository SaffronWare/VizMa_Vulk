# CMakeList.txt : CMake project for VizMa_Vulk, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 4.3)

# Add source to this project's executable.
add_executable (VizMa WIN32 "Main.cpp" )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VizMa PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
