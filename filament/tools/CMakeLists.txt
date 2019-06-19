cmake_minimum_required(VERSION 3.10)
project(ssaogen)

set(TARGET ssaogen)

add_executable(${TARGET} ssaogen.cpp)
target_link_libraries(${TARGET} PRIVATE utils math)
