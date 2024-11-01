cmake_minimum_required(VERSION 3.19)
project(utils)

set(TARGET utils)
set(TARGET_LINUX ${TARGET}/linux)
set(TARGET_GENERIC ${TARGET}/generic)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE PUBLIC_HDRS ${PUBLIC_HDR_DIR}/${TARGET}/*.h)

set(DIST_HDRS
        ${PUBLIC_HDR_DIR}/${TARGET}/algorithm.h
        ${PUBLIC_HDR_DIR}/${TARGET}/bitset.h
        ${PUBLIC_HDR_DIR}/${TARGET}/CallStack.h
        ${PUBLIC_HDR_DIR}/${TARGET}/debug.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Allocator.h
        ${PUBLIC_HDR_DIR}/${TARGET}/BitmaskEnum.h
        ${PUBLIC_HDR_DIR}/${TARGET}/compiler.h
        ${PUBLIC_HDR_DIR}/${TARGET}/compressed_pair.h
        ${PUBLIC_HDR_DIR}/${TARGET}/CString.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Entity.h
        ${PUBLIC_HDR_DIR}/${TARGET}/EntityInstance.h
        ${PUBLIC_HDR_DIR}/${TARGET}/EntityManager.h
        ${PUBLIC_HDR_DIR}/${TARGET}/FixedCapacityVector.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Invocable.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Log.h
        ${PUBLIC_HDR_DIR}/${TARGET}/memalign.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Mutex.h
        ${PUBLIC_HDR_DIR}/${TARGET}/NameComponentManager.h
        ${PUBLIC_HDR_DIR}/${TARGET}/ostream.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Panic.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Path.h
        ${PUBLIC_HDR_DIR}/${TARGET}/PrivateImplementation.h
        ${PUBLIC_HDR_DIR}/${TARGET}/PrivateImplementation-impl.h
        ${PUBLIC_HDR_DIR}/${TARGET}/SingleInstanceComponentManager.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Slice.h
        ${PUBLIC_HDR_DIR}/${TARGET}/StructureOfArrays.h
        ${PUBLIC_HDR_DIR}/${TARGET}/Systrace.h
        ${PUBLIC_HDR_DIR}/${TARGET}/sstream.h
        ${PUBLIC_HDR_DIR}/${TARGET}/unwindows.h
)

# Use our custom mutex/condition only on ANDROID
set(DIST_ANDROID_HDRS
        ${PUBLIC_HDR_DIR}/${TARGET_LINUX}/Mutex.h
)

set(DIST_GENERIC_HDRS
        ${PUBLIC_HDR_DIR}/${TARGET_GENERIC}/Mutex.h
)

set(SRCS
        src/api_level.cpp
        src/architecture.cpp
        src/ashmem.cpp
        src/debug.cpp
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
        src/ostream.cpp
        src/Panic.cpp
        src/Path.cpp
        src/Profiler.cpp
        src/sstream.cpp
        src/string.cpp
        src/ThreadUtils.cpp
)

if (WIN32)
    list(APPEND SRCS src/win32/Path.cpp)
endif()
if (ANDROID)
    list(APPEND SRCS src/android/ThermalManager.cpp)
    list(APPEND SRCS src/android/PerformanceHintManager.cpp)
    list(APPEND SRCS src/android/Systrace.cpp)
endif()
if (LINUX OR ANDROID)
    list(APPEND SRCS src/linux/Condition.cpp)
    list(APPEND SRCS src/linux/Mutex.cpp)
    list(APPEND SRCS src/linux/Path.cpp)
endif()
if (APPLE)
    list(APPEND SRCS src/darwin/Path.mm)
    list(APPEND SRCS src/darwin/Systrace.cpp)
endif()
if (WEBGL)
    list(APPEND SRCS src/web/Path.cpp)
endif()

# ==================================================================================================
# Includes and target definition
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)
target_link_libraries(${TARGET} PUBLIC tsl)

if (ANDROID)
    target_link_libraries(${TARGET} PUBLIC log)
    target_link_libraries(${TARGET} PRIVATE dl)
    target_link_libraries(${TARGET} PUBLIC android)
endif()

if (WIN32)
    # Needed for shlwapi.h (GetModuleFileName)
    target_link_libraries(${TARGET} PUBLIC Shlwapi)
endif()

if (APPLE)
    # Needed for NSTemporaryDirectory()
    target_link_libraries(${TARGET} PRIVATE "-framework Foundation")
endif()

if (LINUX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET} PRIVATE Threads::Threads)
    target_link_libraries(${TARGET} PRIVATE dl)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
set(INSTALL_TYPE ARCHIVE)
install(TARGETS ${TARGET} ${INSTALL_TYPE} DESTINATION lib/${DIST_DIR})
install(FILES ${DIST_HDRS} DESTINATION include/${TARGET})
if (ANDROID)
    install(FILES ${DIST_ANDROID_HDRS} DESTINATION include/${TARGET_LINUX})
else()
    install(FILES ${DIST_GENERIC_HDRS} DESTINATION include/${TARGET_GENERIC})
endif()

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
        test/test_FixedCapacityVector.cpp
        test/test_FixedCircularBuffer.cpp
        test/test_Hash.cpp
        test/test_JobSystem.cpp
        test/test_QuadTreeArray.cpp
        test/test_RangeMap.cpp
        test/test_StructureOfArrays.cpp
        test/test_sstream.cpp
        test/test_string.cpp
        test/test_utils_main.cpp
        test/test_Zip2Iterator.cpp
        test/test_BinaryTreeArray.cpp
)

if (WEBGL_PTHREADS)
    target_compile_definitions(${TARGET} PUBLIC -DFILAMENT_WASM_THREADS)
endif()

# The Path tests are platform-specific
if(FILAMENT_TESTS)
if (NOT WEBGL)
    if (WIN32)
        list(APPEND TEST_SRCS test/test_WinPath.cpp)
    else()
        list(APPEND TEST_SRCS test/test_Path.cpp)
    endif()
endif()

add_executable(test_${TARGET} ${TEST_SRCS})

target_link_libraries(test_${TARGET} PRIVATE gtest utils tsl math)
set_target_properties(test_${TARGET} PROPERTIES FOLDER Tests)
endif()

# ==================================================================================================
# Benchmarks
# ==================================================================================================
if(FILAMENT_BENCHMARKS AND NOT WEBGL)

    add_library(benchmark_${TARGET}_callee SHARED benchmark/benchmark_callee.cpp)
    set_target_properties(benchmark_${TARGET}_callee PROPERTIES FOLDER Benchmarks)

    set(BENCHMARK_SRCS
            benchmark/benchmark_allocators.cpp
            benchmark/benchmark_binary_search.cpp
            benchmark/benchmark_calls.cpp
            benchmark/benchmark_JobSystem.cpp
            benchmark/benchmark_mutex.cpp
            benchmark/benchmark_memcpy.cpp)


    add_executable(benchmark_${TARGET} ${BENCHMARK_SRCS})

    target_link_libraries(benchmark_${TARGET} PRIVATE benchmark_main utils benchmark_${TARGET}_callee)

    set_target_properties(benchmark_${TARGET} PROPERTIES FOLDER Benchmarks)

endif()
