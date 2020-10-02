cmake_minimum_required(VERSION 3.10)
project(bluegl ASM)

set(TARGET              bluegl)
set(PUBLIC_HDR_DIR      include)

if (WIN32)
    enable_language(ASM_MASM)
    set_property(SOURCE src/BlueGLCoreWindowsImpl.S PROPERTY LANGUAGE ASM_MASM)
endif()

# public headers are always in include/${TARGET}/
file(GLOB_RECURSE PUBLIC_HDRS ${PUBLIC_HDR_DIR}/${TARGET}/*.h)

# list each source file individually
set(SRCS
    src/BlueGL.cpp
)

# detect aarch64 (ref: https://stackoverflow.com/a/12024211/1255535)
set(DETECT_ARCH_SRC "
#if defined(__ARMv8_A__) || defined(__ARM64_ARCH_8__) || \
    defined(__aarch64__) || defined(__ARMv8__) || defined(_M_ARM64)
    #define PROCESSOR_ARCH \"aarch64\"
#else
    #define PROCESSOR_ARCH \"other\"
#endif

#include <iostream>
int main() {
    std::cout << PROCESSOR_ARCH;
    return 0;
}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/detect_arch.cpp" "${DETECT_ARCH_SRC}")
try_run(
    run_result                                     # <runResultVar>
    compile_result                                 # <compileResultVar>
    "${CMAKE_CURRENT_BINARY_DIR}"                  # <bindir>
    "${CMAKE_CURRENT_BINARY_DIR}/detect_arch.cpp"  # <srcfile>
    RUN_OUTPUT_VARIABLE PROCESSOR_ARCH)

if (WIN32)
    set(SRCS ${SRCS} src/BlueGLWindows.cpp)
    set(SRCS ${SRCS} src/BlueGLCoreWindowsImpl.S)
elseif (APPLE AND NOT IOS)
    set(SRCS ${SRCS} src/BlueGLDarwin.cpp)
    set(SRCS ${SRCS} src/BlueGLCoreDarwinImpl.S)
elseif(LINUX)
    set(SRCS ${SRCS} src/BlueGLLinux.cpp)
    if ("${PROCESSOR_ARCH}" STREQUAL "aarch64")
        set(SRCS ${SRCS} src/BlueGLCoreLinuxAArch64Impl.S)
    else()
        set(SRCS ${SRCS} src/BlueGLCoreLinuxImpl.S)
    endif()
else()
    message(FATAL_ERROR "Platform not supported. BlueGL supports Windows, Linux, and MacOS X.")
endif()

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()

# specify where our headers are
include_directories(${PUBLIC_HDR_DIR})

# we're building a library
add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

if (WIN32)
    target_link_libraries(${TARGET} PRIVATE opengl32 gdi32)
endif()

install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})

# Build the tests...
add_executable(test_${TARGET}
    tests/OpenGLSupport.cpp
    tests/OpenGLSupport.hpp
    tests/test_bluegl.cpp)

if (LINUX)
    target_link_libraries(test_${TARGET} PUBLIC dl)
endif()

# and we're linking against the libraries below, importing their public headers
target_link_libraries(test_${TARGET} LINK_PUBLIC ${TARGET})
target_link_libraries(test_${TARGET} LINK_PUBLIC gtest)
