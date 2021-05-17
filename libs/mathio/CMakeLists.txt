cmake_minimum_required(VERSION 3.19)
project(math)

set(TARGET mathio)
set(PUBLIC_HDR_DIR include)

if (MSVC)
    set(OPTIMIZATION_FLAGS /fp:fast)
else()
    set(OPTIMIZATION_FLAGS -ffast-math -ffp-contract=fast)
endif()

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/mathio/ostream.h
)

set(SRCS src/ostream.cpp)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_compile_options(${TARGET} PRIVATE ${OPTIMIZATION_FLAGS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
target_link_libraries(${TARGET} PRIVATE math)

# ==================================================================================================
# Installation
# ==================================================================================================
install(DIRECTORY ${PUBLIC_HDR_DIR}/mathio DESTINATION include)

