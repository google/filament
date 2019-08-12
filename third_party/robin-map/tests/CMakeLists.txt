cmake_minimum_required(VERSION 3.8)

project(tsl_robin_map_tests)

add_executable(tsl_robin_map_tests "main.cpp" 
                                   "custom_allocator_tests.cpp"
                                   "policy_tests.cpp"
                                   "robin_map_tests.cpp" 
                                   "robin_set_tests.cpp")

target_compile_features(tsl_robin_map_tests PRIVATE cxx_std_11)
                                    
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(tsl_robin_map_tests PRIVATE -Werror -Wall -Wextra -Wold-style-cast -DTSL_DEBUG -UNDEBUG)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(tsl_robin_map_tests PRIVATE /bigobj /WX /W3 /DTSL_DEBUG /UNDEBUG)
endif()

# Boost::unit_test_framework
find_package(Boost 1.54.0 REQUIRED COMPONENTS unit_test_framework)
target_link_libraries(tsl_robin_map_tests PRIVATE Boost::unit_test_framework)   

# tsl::robin_map
add_subdirectory(../ ${CMAKE_CURRENT_BINARY_DIR}/tsl)
target_link_libraries(tsl_robin_map_tests PRIVATE tsl::robin_map)  
