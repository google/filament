add_library(SPIRV-Headers-example
	${CMAKE_CURRENT_SOURCE_DIR}/example.cpp)
target_include_directories(SPIRV-Headers-example
	PRIVATE ${SPIRV-Headers_SOURCE_DIR}/include)
