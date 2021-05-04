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

if (WIN32)
    set(SRCS ${SRCS} src/BlueGLWindows.cpp)
    set(SRCS ${SRCS} src/BlueGLCoreWindowsImpl.S)
elseif (APPLE AND NOT IOS)
    set(SRCS ${SRCS} src/BlueGLDarwin.cpp)
    set(SRCS ${SRCS} src/BlueGLCoreDarwinUniversalImpl.S)
elseif(LINUX)
    set(SRCS ${SRCS} src/BlueGLLinux.cpp)
    set(SRCS ${SRCS} src/BlueGLCoreLinuxUniversalImpl.S)
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
if(HUNTER_ENABLED)
  target_include_directories(${TARGET} PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>  
    $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>)
else()
  target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
endif()

if (WIN32)
    target_link_libraries(${TARGET} PRIVATE opengl32 gdi32)
endif()

if(HUNTER_ENABLED)
  install(TARGETS ${TARGET} EXPORT ${TARGETS_EXPORT_NAME} ARCHIVE DESTINATION lib/${DIST_DIR})
else()
  install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
endif()

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
if(HUNTER_ENABLED)
  target_link_libraries(test_${TARGET} LINK_PUBLIC GTest::gtest GTest::gtest_main Threads::Threads)
else()
  target_link_libraries(test_${TARGET} LINK_PUBLIC gtest)
endif()
