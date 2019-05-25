#===============================================================================
# Copyright 2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#===============================================================================

# Manage different library options
#===============================================================================

if(options_cmake_included)
    return()
endif()
set(options_cmake_included true)

# ========
# Features
# ========

option(MKLDNN_VERBOSE
    "allows Intel(R) MKL-DNN be verbose whenever MKLDNN_VERBOSE
    environment variable set to 1" ON) # enabled by default

option(MKLDNN_ENABLE_CONCURRENT_EXEC
    "disables sharing a common scratchpad between primitives.
    This option must be turned on if there is a possibility of concurrent
    execution of primitives that were created in the same thread.
    CAUTION: enabling this option increases memory consumption"
    OFF) # disabled by default

# =============================
# Building properties and scope
# =============================

set(MKLDNN_LIBRARY_TYPE "SHARED" CACHE STRING
    "specifies whether Intel(R) MKL-DNN library should be SHARED or STATIC")
option(MKLDNN_BUILD_EXAMPLES "builds examples"  ON)
option(MKLDNN_BUILD_TESTS "builds tests" ON)

set(MKLDNN_THREADING "OMP" CACHE STRING
    "specifies threading type; supports OMP (default), OMP:COMP, OMP:INTEL, or TBB.

    When OpenMP is used a user can choose what runtime to use:
    - native OpenMP runtime that comes with the compiler (OMP:COMP), or
    - Intel OpenMP runtime that is compatible with all the compilers that
      Intel MKL-DNN supports (OMP:INTEL). This option requires Intel MKL
      be installed or Intel MKL-ML library be downloaded. This option doesn't
      work with MSVC (w/o Intel Compiler).
    The default option is OMP, which gives a preference to OMP:INTEL, but if
    neither Intel MKL is installed nor Intel MKL-ML is available then fallback
    to OMP:COMP.

    To use Intel(R) Threading Building Blocks (Intel(R) TBB) one should also
    set TBBROOT (either environment variable or CMake option) to the library
    location")

set(MKLDNN_USE_MKL "DEF" CACHE STRING
    "specifies what Intel MKL library to use.
    Supports DEF (default), NONE, ML, FULL, FULL:STATIC.

    By default (DEF) cmakes tries to find Intel MKL-ML library, then full
    Intel MKL library, or just builds Intel MKL-DNN w/o any binary dependency.

    To build Intel MKL-DNN w/o any dependencies on Intel MKL / Intel MKL-ML
    use NONE. Note that building system would not be able to use Intel OpenMP
    runtime that comes with Intel MKL or Intel MKL-ML, and would be available
    only if Intel Compiler is used.

    To force Intel MKL-DNN to use Intel MKL-ML use ML. Depending on the
    threading the build system would choose between libmklml_intel or
    libmklml_gnu.

    To force Intel MKL-DNN to use the full Intel MKL pass FULL or FULL:STATIC
    to cmake. The former option would make Intel MKL-DNN link against
    Intel MKL RT (libmkl_rt). The latter one would link against static
    Intel MKL. Use static linking to reduce the size of the resulting library
    (including its dependencies).
    Caution: Intel MKL RT allows setting the threading layer using environment
             variable MKL_THREADING_LAYER. By default Intel MKL would use
             OpenMP. If Intel MKL-DNN is built with TBB it is recommended to
             set MKL_THREADING_LAYER to `tbb` or `sequential`, to avoid
             conflict between OpenMP and TBB thread pools.")

# ======================
# Profiling capabilities
# ======================

option(MKLDNN_ENABLE_JIT_PROFILING
    "Enable registration of Intel(R) MKL-DNN kernels that are generated at
    runtime with Intel VTune Amplifier (on by default). Without the
    registrations, Intel VTune Amplifier would report data collected inside
    the kernels as `outside any known module`."
    ON)

# =============
# Miscellaneous
# =============

option(BENCHDNN_USE_RDPMC
    "enables rdpms counter to report precise cpu frequency in benchdnn.
     CAUTION: may not work on all cpus (hence disabled by default)"
    OFF) # disabled by default

# =============
# Developer flags
# =============

set(MKLDNN_USE_CLANG_SANITIZER "" CACHE STRING
    "instructs build system to use a Clang sanitizer. Possible values:
    Address: enables AddressSanitizer
    Memory: enables MemorySanitizer
    MemoryWithOrigin: enables MemorySanitizer with origin tracking
    Undefined: enables UndefinedBehaviourSanitizer
    This feature is experimental and is only available on Linux.")

option(MKLDNN_PRODUCT_BUILD_MODE
    "Enables/disables product build mode. For example,
    setting MKLDNN_PRODUCT_BUILD_MODE=OFF makes warnings non-fatal"
    ON)
