cmake_minimum_required(VERSION 3.3)
project(OpenCLExamples)

set(OPENCL_EXAMPLES 
	MatrixMultiplication
)

foreach(EXAMPLE ${OPENCL_EXAMPLES})
	message ("==>CMake generation for OpenCL/${EXAMPLE}...")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE}")
endforeach()