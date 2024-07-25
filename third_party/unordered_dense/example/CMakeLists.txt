cmake_minimum_required(VERSION 3.12)
project("UnorderedDenseExample")

add_executable(UnorderedDenseExample main.cpp)
find_package(unordered_dense CONFIG REQUIRED)
target_link_libraries(UnorderedDenseExample unordered_dense::unordered_dense)
