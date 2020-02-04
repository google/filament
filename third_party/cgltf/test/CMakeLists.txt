cmake_minimum_required( VERSION 2.8 )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set( EXE_NAME cgltf_test )
add_executable( ${EXE_NAME} main.c )
set_property( TARGET ${EXE_NAME} PROPERTY C_STANDARD 99 )
install( TARGETS ${EXE_NAME} RUNTIME DESTINATION bin )

set( EXE_NAME test_conversion )
add_executable( ${EXE_NAME} test_conversion.cpp )
set_property( TARGET ${EXE_NAME} PROPERTY CXX_STANDARD 11 )
install( TARGETS ${EXE_NAME} RUNTIME DESTINATION bin )

set( EXE_NAME test_write )
add_executable( ${EXE_NAME} test_write.cpp )
set_property( TARGET ${EXE_NAME} PROPERTY CXX_STANDARD 11 )
install( TARGETS ${EXE_NAME} RUNTIME DESTINATION bin )

set( EXE_NAME test_math )
add_executable( ${EXE_NAME} test_math.cpp )
set_property( TARGET ${EXE_NAME} PROPERTY CXX_STANDARD 11 )
install( TARGETS ${EXE_NAME} RUNTIME DESTINATION bin )
