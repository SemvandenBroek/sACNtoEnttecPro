# CMakeList.txt : CMake project for sACNtoOpenDmx, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("sACNtoOpenDmx")

# Add source to this project's executable.
add_executable (sACNtoOpenDmx "sACNtoOpenDmx.cpp" "sACNtoOpenDmx.h")

# Add sACN library
add_subdirectory("lib/sACN")
target_link_libraries(sACNtoOpenDmx PRIVATE sACN)

# TODO: Add tests and install targets if needed.
