# Copyright 2019 The Marl Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.0)

include(cmake/parse_version.cmake)
parse_version("${CMAKE_CURRENT_SOURCE_DIR}/CHANGES.md" MARL)

set(CMAKE_CXX_STANDARD 11)

project(Marl
    VERSION   "${MARL_VERSION_MAJOR}.${MARL_VERSION_MINOR}.${MARL_VERSION_PATCH}"
    LANGUAGES C CXX ASM
)

if (EMSCRIPTEN)
    add_compile_options(-O3 -pthread)
endif()

include(CheckCXXSourceCompiles)

# MARL_IS_SUBPROJECT is 1 if added via add_subdirectory() from another project.
get_directory_property(MARL_IS_SUBPROJECT PARENT_DIRECTORY)
if(MARL_IS_SUBPROJECT)
    set(MARL_IS_SUBPROJECT 1)
endif()

###########################################################
# Options
###########################################################
function(option_if_not_defined name description default)
    if(NOT DEFINED ${name})
        option(${name} ${description} ${default})
    endif()
endfunction()

option_if_not_defined(MARL_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option_if_not_defined(MARL_BUILD_EXAMPLES "Build example applications" OFF)
option_if_not_defined(MARL_BUILD_TESTS "Build tests" OFF)
option_if_not_defined(MARL_BUILD_BENCHMARKS "Build benchmarks" OFF)
option_if_not_defined(MARL_BUILD_SHARED "Build marl as a shared / dynamic library (default static)" OFF)
option_if_not_defined(MARL_USE_PTHREAD_THREAD_LOCAL "Use pthreads for thread local storage" OFF)
option_if_not_defined(MARL_ASAN "Build marl with address sanitizer" OFF)
option_if_not_defined(MARL_MSAN "Build marl with memory sanitizer" OFF)
option_if_not_defined(MARL_TSAN "Build marl with thread sanitizer" OFF)
option_if_not_defined(MARL_UBSAN "Build marl with undefined-behavior sanitizer" OFF)
option_if_not_defined(MARL_INSTALL "Create marl install target" OFF)
option_if_not_defined(MARL_FULL_BENCHMARK "Run benchmarks for [0 .. numLogicalCPUs] with no stepping" OFF)
option_if_not_defined(MARL_FIBERS_USE_UCONTEXT "Use ucontext instead of assembly for fibers (ignored for platforms that do not support ucontext)" OFF)
option_if_not_defined(MARL_DEBUG_ENABLED "Enable debug checks even in release builds" OFF)

###########################################################
# Directories
###########################################################
function(set_if_not_defined name value)
    if(NOT DEFINED ${name})
        set(${name} ${value} PARENT_SCOPE)
    endif()
endfunction()

set(MARL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MARL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_if_not_defined(MARL_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set_if_not_defined(MARL_GOOGLETEST_DIR ${MARL_THIRD_PARTY_DIR}/googletest)
set_if_not_defined(MARL_BENCHMARK_DIR ${MARL_THIRD_PARTY_DIR}/benchmark)

###########################################################
# Submodules
###########################################################
if(MARL_BUILD_TESTS)
    if(NOT EXISTS ${MARL_GOOGLETEST_DIR}/.git)
        message(WARNING "third_party/googletest submodule missing.")
        message(WARNING "Run: `git submodule update --init` to build tests.")
        set(MARL_BUILD_TESTS OFF)
    endif()
endif(MARL_BUILD_TESTS)

if(MARL_BUILD_BENCHMARKS)
    if(NOT EXISTS ${MARL_BENCHMARK_DIR}/.git)
        message(WARNING "third_party/benchmark submodule missing.")
        message(WARNING "Run: `git submodule update --init` to build benchmarks.")
        set(MARL_BUILD_BENCHMARKS OFF)
    endif()
endif(MARL_BUILD_BENCHMARKS)

if(MARL_BUILD_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING FALSE CACHE BOOL FALSE FORCE)
    add_subdirectory(${MARL_BENCHMARK_DIR})
endif(MARL_BUILD_BENCHMARKS)

###########################################################
# Compiler feature tests
###########################################################
# Check that the Clang Thread Safety Analysis' try_acquire_capability behaves
# correctly. This is broken on some earlier versions of clang.
# See: https://bugs.llvm.org/show_bug.cgi?id=32954
set(SAVE_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_FLAGS "-Wthread-safety -Werror")
check_cxx_source_compiles(
    "int main() {
      struct __attribute__((capability(\"mutex\"))) Mutex {
        void Unlock() __attribute__((release_capability)) {};
        bool TryLock() __attribute__((try_acquire_capability(true))) { return true; };
      };
      Mutex m;
      if (m.TryLock()) {
        m.Unlock();  // Should not warn.
      }
      return 0;
    }"
    MARL_THREAD_SAFETY_ANALYSIS_SUPPORTED)
set(CMAKE_REQUIRED_FLAGS ${SAVE_CMAKE_REQUIRED_FLAGS})

# Check whether ucontext is supported.
set(SAVE_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_FLAGS "-Werror")
check_cxx_source_compiles(
    "#include <ucontext.h>
    int main() {
      ucontext_t ctx;
      getcontext(&ctx);
      makecontext(&ctx, nullptr, 2, 1, 2);
      swapcontext(&ctx, &ctx);
      return 0;
    }"
    MARL_UCONTEXT_SUPPORTED)
set(CMAKE_REQUIRED_FLAGS ${SAVE_CMAKE_REQUIRED_FLAGS})
if (MARL_FIBERS_USE_UCONTEXT AND NOT MARL_UCONTEXT_SUPPORTED)
    # Disable MARL_FIBERS_USE_UCONTEXT and warn if MARL_UCONTEXT_SUPPORTED is 0.
    message(WARNING "MARL_FIBERS_USE_UCONTEXT is enabled, but ucontext is not supported by the target. Disabling")
    set(MARL_FIBERS_USE_UCONTEXT 0)
endif()

if(MARL_IS_SUBPROJECT)
    # Export supported flags as this may be useful to parent projects
    set(MARL_THREAD_SAFETY_ANALYSIS_SUPPORTED PARENT_SCOPE ${MARL_THREAD_SAFETY_ANALYSIS_SUPPORTED})
    set(MARL_UCONTEXT_SUPPORTED               PARENT_SCOPE ${MARL_UCONTEXT_SUPPORTED})
endif()

###########################################################
# File lists
###########################################################
set(MARL_LIST
    ${MARL_SRC_DIR}/debug.cpp
    ${MARL_SRC_DIR}/memory.cpp
    ${MARL_SRC_DIR}/scheduler.cpp
    ${MARL_SRC_DIR}/thread.cpp
    ${MARL_SRC_DIR}/trace.cpp
)
if(NOT MSVC)
    list(APPEND MARL_LIST
        ${MARL_SRC_DIR}/osfiber_aarch64.c
        ${MARL_SRC_DIR}/osfiber_arm.c
        ${MARL_SRC_DIR}/osfiber_asm_aarch64.S
        ${MARL_SRC_DIR}/osfiber_asm_arm.S
        ${MARL_SRC_DIR}/osfiber_asm_loongarch64.S
        ${MARL_SRC_DIR}/osfiber_asm_mips64.S
        ${MARL_SRC_DIR}/osfiber_asm_ppc64.S
        ${MARL_SRC_DIR}/osfiber_asm_rv64.S
        ${MARL_SRC_DIR}/osfiber_asm_x64.S
        ${MARL_SRC_DIR}/osfiber_asm_x86.S
        ${MARL_SRC_DIR}/osfiber_loongarch64.c
        ${MARL_SRC_DIR}/osfiber_mips64.c
        ${MARL_SRC_DIR}/osfiber_ppc64.c
        ${MARL_SRC_DIR}/osfiber_rv64.c
        ${MARL_SRC_DIR}/osfiber_x64.c
        ${MARL_SRC_DIR}/osfiber_x86.c
        ${MARL_SRC_DIR}/osfiber_emscripten.cpp
    )
    # CMAKE_OSX_ARCHITECTURES settings aren't propagated to assembly files when
    # building for Apple platforms (https://gitlab.kitware.com/cmake/cmake/-/issues/20771),
    # we treat assembly files as C files to work around this bug.
    set_source_files_properties(
        ${MARL_SRC_DIR}/osfiber_asm_aarch64.S
        ${MARL_SRC_DIR}/osfiber_asm_arm.S
        ${MARL_SRC_DIR}/osfiber_asm_loongarch64.S
        ${MARL_SRC_DIR}/osfiber_asm_mips64.S
        ${MARL_SRC_DIR}/osfiber_asm_ppc64.S
        ${MARL_SRC_DIR}/osfiber_asm_x64.S
        ${MARL_SRC_DIR}/osfiber_asm_x86.S
        PROPERTIES LANGUAGE C
    )
endif(NOT MSVC)

###########################################################
# OS libraries
###########################################################
find_package(Threads REQUIRED)

###########################################################
# Functions
###########################################################
function(marl_set_target_options target)
    if(MARL_THREAD_SAFETY_ANALYSIS_SUPPORTED)
        target_compile_options(${target} PRIVATE "-Wthread-safety")
    endif()

    # Enable all warnings
    if(MSVC)
        target_compile_options(${target} PRIVATE "-W4")
    else()
        target_compile_options(${target} PRIVATE "-Wall")
    endif()

    # Disable specific, pedantic warnings
    if(MSVC)
        target_compile_options(${target} PRIVATE
            "-D_CRT_SECURE_NO_WARNINGS"
            "/wd4127" # conditional expression is constant
            "/wd4324" # structure was padded due to alignment specifier
        )
    endif()

    # Treat all warnings as errors
    if(MARL_WARNINGS_AS_ERRORS)
        if(MSVC)
            target_compile_options(${target} PRIVATE "/WX")
        else()
            target_compile_options(${target} PRIVATE "-Werror")
        endif()
    endif(MARL_WARNINGS_AS_ERRORS)

    if(MARL_USE_PTHREAD_THREAD_LOCAL)
        target_compile_definitions(${target} PRIVATE "MARL_USE_PTHREAD_THREAD_LOCAL=1")
        target_link_libraries(${target} PUBLIC pthread)
    endif()

    if(MARL_ASAN)
        target_compile_options(${target} PUBLIC "-fsanitize=address")
        target_link_libraries(${target} PUBLIC "-fsanitize=address")
    elseif(MARL_MSAN)
        target_compile_options(${target} PUBLIC "-fsanitize=memory")
        target_link_libraries(${target} PUBLIC "-fsanitize=memory")
    elseif(MARL_TSAN)
        target_compile_options(${target} PUBLIC "-fsanitize=thread")
        target_link_libraries(${target} PUBLIC "-fsanitize=thread")
    elseif(MARL_UBSAN)
        target_compile_options(${target} PUBLIC "-fsanitize=undefined")
        target_link_libraries(${target} PUBLIC "-fsanitize=undefined")
    endif()

    if(MARL_FIBERS_USE_UCONTEXT)
        target_compile_definitions(${target} PRIVATE "MARL_FIBERS_USE_UCONTEXT=1")
    endif()

    if(MARL_DEBUG_ENABLED)
        target_compile_definitions(${target} PRIVATE "MARL_DEBUG_ENABLED=1")
    endif()

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^rv.*")
        target_link_libraries(${target} INTERFACE atomic) #explicitly use -latomic for RISC-V linking
    endif()

    target_include_directories(${target} PUBLIC $<BUILD_INTERFACE:${MARL_INCLUDE_DIR}>)
endfunction(marl_set_target_options)

###########################################################
# Targets
###########################################################

# marl
if(MARL_BUILD_SHARED OR BUILD_SHARED_LIBS)
    add_library(marl SHARED ${MARL_LIST})
    if(MSVC)
        target_compile_definitions(marl
            PRIVATE "MARL_BUILDING_DLL=1"
            PUBLIC  "MARL_DLL=1"
        )
    endif()
else()
    add_library(marl ${MARL_LIST})
endif()

if(NOT MSVC)
    # Public API symbols are made visible with the MARL_EXPORT annotation.
    target_compile_options(marl PRIVATE "-fvisibility=hidden")
endif()

set_target_properties(marl PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    VERSION ${MARL_VERSION}
    SOVERSION "${MARL_VERSION_MAJOR}"
)

marl_set_target_options(marl)

target_link_libraries(marl PUBLIC Threads::Threads)

# install
if(MARL_INSTALL)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/marl-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/marl-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/marl
    )

    install(DIRECTORY ${MARL_INCLUDE_DIR}/marl
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        USE_SOURCE_PERMISSIONS
    )

    install(TARGETS marl
        EXPORT marl-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT marl-targets
        FILE marl-targets.cmake
        NAMESPACE marl::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/marl
    )

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/marl-config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/marl
    )
endif(MARL_INSTALL)

# tests
if(MARL_BUILD_TESTS)
    set(MARL_TEST_LIST
        ${MARL_SRC_DIR}/blockingcall_test.cpp
        ${MARL_SRC_DIR}/conditionvariable_test.cpp
        ${MARL_SRC_DIR}/containers_test.cpp
        ${MARL_SRC_DIR}/dag_test.cpp
        ${MARL_SRC_DIR}/defer_test.cpp
        ${MARL_SRC_DIR}/event_test.cpp
        ${MARL_SRC_DIR}/marl_test.cpp
        ${MARL_SRC_DIR}/marl_test.h
        ${MARL_SRC_DIR}/memory_test.cpp
        ${MARL_SRC_DIR}/osfiber_test.cpp
        ${MARL_SRC_DIR}/parallelize_test.cpp
        ${MARL_SRC_DIR}/pool_test.cpp
        ${MARL_SRC_DIR}/scheduler_test.cpp
        ${MARL_SRC_DIR}/thread_test.cpp
        ${MARL_SRC_DIR}/ticket_test.cpp
        ${MARL_SRC_DIR}/waitgroup_test.cpp
        ${MARL_GOOGLETEST_DIR}/googletest/src/gtest-all.cc
        ${MARL_GOOGLETEST_DIR}/googlemock/src/gmock-all.cc
    )

    set(MARL_TEST_INCLUDE_DIR
        ${MARL_GOOGLETEST_DIR}/googletest/include/
        ${MARL_GOOGLETEST_DIR}/googlemock/include/
        ${MARL_GOOGLETEST_DIR}/googletest/
        ${MARL_GOOGLETEST_DIR}/googlemock/
    )

    add_executable(marl-unittests ${MARL_TEST_LIST})

    set_target_properties(marl-unittests PROPERTIES
        INCLUDE_DIRECTORIES "${MARL_TEST_INCLUDE_DIR}"
        FOLDER "Tests"
    )

    marl_set_target_options(marl-unittests)

    target_link_libraries(marl-unittests PRIVATE marl)
endif(MARL_BUILD_TESTS)

# benchmarks
if(MARL_BUILD_BENCHMARKS)
    set(MARL_BENCHMARK_LIST
        ${MARL_SRC_DIR}/blockingcall_bench.cpp
        ${MARL_SRC_DIR}/defer_bench.cpp
        ${MARL_SRC_DIR}/event_bench.cpp
        ${MARL_SRC_DIR}/marl_bench.cpp
        ${MARL_SRC_DIR}/non_marl_bench.cpp
        ${MARL_SRC_DIR}/scheduler_bench.cpp
        ${MARL_SRC_DIR}/ticket_bench.cpp
        ${MARL_SRC_DIR}/waitgroup_bench.cpp
    )

    add_executable(marl-benchmarks ${MARL_BENCHMARK_LIST})
    set_target_properties(${target} PROPERTIES FOLDER "Benchmarks")

    marl_set_target_options(marl-benchmarks)

    target_compile_definitions(marl-benchmarks PRIVATE
        "MARL_FULL_BENCHMARK=${MARL_FULL_BENCHMARK}"
    )

    target_link_libraries(marl-benchmarks PRIVATE benchmark::benchmark marl)
endif(MARL_BUILD_BENCHMARKS)

# examples
if(MARL_BUILD_EXAMPLES)
    function(build_example target)
        add_executable(${target} "${CMAKE_CURRENT_SOURCE_DIR}/examples/${target}.cpp")
        set_target_properties(${target} PROPERTIES FOLDER "Examples")
        marl_set_target_options(${target})
        target_link_libraries(${target} PRIVATE marl)
        if (EMSCRIPTEN)
            target_link_options(${target} PRIVATE
                    -O1
                    -pthread -sPTHREAD_POOL_SIZE=2 -sPROXY_TO_PTHREAD
                    -sASYNCIFY # -sASYNCIFY_STACK_SIZE=1000000
                    -sALLOW_MEMORY_GROWTH=1 -sASSERTIONS
                    -sENVIRONMENT=web,worker
                    "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/examples/shell.emscripten.html")
            set_target_properties(${target} PROPERTIES SUFFIX .html)
        endif()
    endfunction(build_example)

    build_example(fractal)
    build_example(hello_task)
    build_example(primes)
    build_example(tasks_in_tasks)
endif(MARL_BUILD_EXAMPLES)
