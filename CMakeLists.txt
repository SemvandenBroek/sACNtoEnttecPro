# CMakeList.txt : CMake project for sACNtoOpenDmx, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("sACNtoOpenDmx")

# Add source to this project's executable.
add_executable (sACNtoOpenDmx WIN32 "sACNtoOpenDmx.cpp" "sACNtoOpenDmx.h" "resource.h" "resource.rc" "app.cpp" "app.h" "usb_pro.cpp" )
set_source_files_properties(resource.rc PROPERTIES LANGUAGE RC)

# Add sACN library
add_subdirectory("lib/sACN")
target_link_libraries(sACNtoOpenDmx PRIVATE sACN)
target_link_libraries(sACNtoOpenDmx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/usb_pro/ftd2xx.lib)

# TODO: Add tests and install targets if needed.
