cmake_minimum_required(VERSION 3.19)
project(angle)

add_library(angle INTERFACE)
target_include_directories(angle INTERFACE ${ANGLE_INCLUDE_DIR})
