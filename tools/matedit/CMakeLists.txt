cmake_minimum_required(VERSION 3.19)
project(matedit)

set(TARGET matedit)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(SRCS
    src/main.cpp
    src/ExternalCompile.cpp
)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${SRCS})

target_include_directories(${TARGET} PRIVATE ${filamat_SOURCE_DIR}/src)

target_link_libraries(${TARGET} matdbg getopt)

set_target_properties(${TARGET} PROPERTIES FOLDER Tools)

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
