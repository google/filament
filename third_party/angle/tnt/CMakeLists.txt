cmake_minimum_required(VERSION 3.19)
project(angle)

add_library(angle INTERFACE)
target_include_directories(angle INTERFACE ${ANGLE_INCLUDE_DIR})

# Uncomment the following lines and specify your folder containing ANGLE libraries
# to enable linking sample projects using ANGLE. You have to do this with the root
# CMakeLists.txt of the `samples` folder too.
# set(ANGLE_LIBS_DIR "")
# target_link_directories(angle INTERFACE ${ANGLE_LIBS_DIR})
# target_link_libraries(angle INTERFACE "libEGL.dll.lib" "libGLESv2.dll.lib")