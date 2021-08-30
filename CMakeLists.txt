# CMakeList.txt : CMake project for sACNtoOpenDmx, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("sACNtoOpenDmx")

# Add source to this project's executable.
add_executable (sACNtoOpenDmx "sACNtoOpenDmx.cpp" "sACNtoOpenDmx.h")

# TODO: Add tests and install targets if needed.
