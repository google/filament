cmake_minimum_required(VERSION 3.1)
project(image)

set(TARGET image)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/image/Image.h
        include/image/utilities.h
)

set(SRCS
        src/Image.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC math)

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (NOT MSVC)
	target_compile_options(${TARGET} PRIVATE
    	    -Wno-deprecated-register
        	$<$<CONFIG:Release>:-ffast-math>
	)
endif()