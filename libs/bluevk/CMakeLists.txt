cmake_minimum_required(VERSION 3.10)
project(bluevk)

set(TARGET              bluevk)
set(PUBLIC_HDR_DIR      include)

file(GLOB_RECURSE PUBLIC_HDRS ${PUBLIC_HDR_DIR}/${TARGET}/*.h)

set(SRCS src/BlueVK.cpp)

# OS specific sources
if (WIN32)
    set(SRCS ${SRCS} src/BlueVKWindows.cpp)
elseif (APPLE)
    set(SRCS ${SRCS} src/BlueVKDarwin.cpp)
elseif (LINUX OR ANDROID)
    set(SRCS ${SRCS} src/BlueVKLinuxAndroid.cpp)
else()
    message(FATAL_ERROR "Platform not supported. BlueVK supports Linux, Android, MacOS X, and Windows.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} utils math)

if(HUNTER_ENABLED)
  target_include_directories(${TARGET} PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>  
    $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>)
else()
  target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
endif()

# test_bluevk is not supported on mobile or Windows
if (NOT ANDROID AND NOT IOS AND NOT WIN32)
    add_executable(test_bluevk tests/test_bluevk_sdl.cpp)
    if(HUNTER_ENABLED)
    target_link_libraries(test_bluevk PRIVATE dl bluevk SDL2::SDL2)
    else ()
      target_link_libraries(test_bluevk PRIVATE dl bluevk sdl2)
    endif()
endif()

if(HUNTER_ENABLED)
  install(TARGETS ${TARGET} EXPORT ${TARGETS_EXPORT_NAME} ARCHIVE DESTINATION lib/${DIST_DIR})
else ()
  install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
endif()
