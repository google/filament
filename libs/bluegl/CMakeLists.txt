cmake_minimum_required(VERSION 3.19)
project(bluegl ASM)

set(TARGET              bluegl)
set(PUBLIC_HDR_DIR      include)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(IS_64_BIT TRUE)
else()
    set(IS_64_BIT FALSE)
endif()

if (WIN32 AND IS_64_BIT)
    enable_language(ASM_MASM)
    set_property(SOURCE src/BlueGLCoreWindowsImpl.S PROPERTY LANGUAGE ASM_MASM)
endif()

# public headers are always in include/${TARGET}/
file(GLOB_RECURSE PUBLIC_HDRS ${PUBLIC_HDR_DIR}/${TARGET}/*.h)

# list each source file individually
set(SRCS
    src/BlueGL.cpp
)

if (WIN32)
    set(SRCS ${SRCS} src/BlueGLWindows.cpp)
    if(NOT IS_64_BIT)
        set(SRCS ${SRCS} src/BlueGLCoreWindows32Impl.cpp)
    else()
        set(SRCS ${SRCS} src/BlueGLCoreWindowsImpl.S)
    endif()
elseif (APPLE AND NOT IOS)
    set(SRCS ${SRCS} src/BlueGLDarwin.cpp)
    set(SRCS ${SRCS} src/BlueGLCoreDarwinUniversalImpl.S)
elseif(LINUX)
    if(FILAMENT_SUPPORTS_EGL_ON_LINUX)
        set(SRCS ${SRCS} src/BlueGLLinuxEGL.cpp)
    else()
        set(SRCS ${SRCS} src/BlueGLLinux.cpp)
    endif()
    set(SRCS ${SRCS} src/BlueGLCoreLinuxUniversalImpl.S)
else()
    message(FATAL_ERROR "Platform not supported. BlueGL supports Windows, Linux, and MacOS X.")
endif()

# specify where our headers are
include_directories(${PUBLIC_HDR_DIR})

# we're building a library
add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

if (WIN32)
    target_link_libraries(${TARGET} PRIVATE opengl32 gdi32)
endif()

install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})

# Build the tests...
if(FILAMENT_TESTS)
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
set_target_properties(test_${TARGET} PROPERTIES FOLDER Tests)
endif()
