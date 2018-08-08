cmake_minimum_required(VERSION 3.1)
project(utils)

set(TARGET utils)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE PUBLIC_HDRS ${PUBLIC_HDR_DIR}/${TARGET}/*.h)

set(DIST_HDRS
        ${PUBLIC_HDR_DIR}/${TARGET}/algorithm.h
        ${PUBLIC_HDR_DIR}/${TARGET}/bitset.h
        ${PUBLIC_HDR_DIR}/${TARGET}/compiler.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Entity.h
        ${PUBLIC_HDR_DIR}/${TARGET}/EntityInstance.h
        ${PUBLIC_HDR_DIR}/${TARGET}/EntityManager.h
        ${PUBLIC_HDR_DIR}/${TARGET}/CString.h
)

set(SRCS
        src/ashmem.cpp
        src/Allocator.cpp
        src/CallStack.cpp
        src/CString.cpp
        src/CountDownLatch.cpp
        src/CyclicBarrier.cpp
        src/EntityManager.cpp
        src/EntityManagerImpl.h
        src/JobSystem.cpp
        src/Log.cpp
        src/NameComponentManager.cpp
        src/Panic.cpp
        src/Path.cpp
        src/Profiler.cpp
        src/Systrace.cpp
        src/linux/futex.cpp
)
if (WIN32)
    list(APPEND SRCS src/win32/Path.cpp)
endif()
if (LINUX OR ANDROID)
    list(APPEND SRCS src/linux/Path.cpp)
endif()
if (APPLE)
    list(APPEND SRCS src/darwin/Path.cpp)
endif()

# ==================================================================================================
# Includes and target definition
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
target_link_libraries(${TARGET} PUBLIC tsl)

if (ANDROID)
    target_link_libraries(${TARGET} PUBLIC log)
endif()

if (WIN32)
    # Needed for shlwapi.h (GetModuleFileName)
    target_link_libraries(${TARGET} PUBLIC Shlwapi)
endif()

if (LINUX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET} PRIVATE Threads::Threads)
endif()

# ==================================================================================================
# Compiler flags
# ==================================================================================================
target_compile_options(${TARGET} PRIVATE
# TODO: use hidden by default and expose what we need
#        -fvisibility=hidden
#        -fvisibility-inlines-hidden
        $<$<PLATFORM_ID:Linux>:-fPIC>
)

# ==================================================================================================
# Installation
# ==================================================================================================
set(INSTALL_TYPE ARCHIVE)
install(TARGETS ${TARGET} ${INSTALL_TYPE} DESTINATION lib/${DIST_DIR})
install(FILES ${DIST_HDRS} DESTINATION include/${TARGET})

# ==================================================================================================
# Test executables
# ==================================================================================================

set(TEST_SRCS
        test/test_algorithm.cpp
        test/test_Allocators.cpp
        test/test_bitset.cpp
        test/test_CountDownLatch.cpp
        test/test_CString.cpp
        test/test_CyclicBarrier.cpp
        test/test_Entity.cpp
        test/test_JobSystem.cpp
        test/test_StructureOfArrays.cpp
        test/test_utils_main.cpp
        test/test_Zip2Iterator.cpp
)

# The Path tests are platform-specific
if (WIN32)
    list(APPEND TEST_SRCS test/test_WinPath.cpp)
else()
    list(APPEND TEST_SRCS test/test_Path.cpp)
endif()

add_executable(test_${TARGET} ${TEST_SRCS})

target_link_libraries(test_${TARGET} PRIVATE gtest utils tsl math)
