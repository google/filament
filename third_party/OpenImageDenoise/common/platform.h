// ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#if defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <Windows.h>
#elif defined(__APPLE__)
  #include <sys/sysctl.h>
#endif

#include <xmmintrin.h>
#include <cstdint>
#include <climits>
#include <atomic>
#include <algorithm>
#include <memory>
#include <cmath>
#include <string>
#include <iostream>
#include <cassert>
#include "include/OpenImageDenoise/oidn.hpp"

// ----------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------

#if defined(_WIN32)
  // Windows
  #if !defined(__noinline)
    #define __noinline     __declspec(noinline)
  #endif
#else
  // Unix
  #if !defined(__forceinline)
    #define __forceinline  inline __attribute__((always_inline))
  #endif
  #if !defined(__noinline)
    #define __noinline     __attribute__((noinline))
  #endif
#endif

#ifndef UNUSED
  #define UNUSED(x) ((void)x)
#endif
#ifndef MAYBE_UNUSED
  #define MAYBE_UNUSED(x) UNUSED(x)
#endif

// ----------------------------------------------------------------------------
// Error handling and debugging
// ----------------------------------------------------------------------------

#define OIDN_WARNING(message) { std::cerr << "Warning: " << message << std::endl << std::flush; }
#define OIDN_FATAL(message)   throw std::runtime_error(message);

// ----------------------------------------------------------------------------
// Common functions
// ----------------------------------------------------------------------------

namespace oidn {

  using std::min;
  using std::max;

  template<typename T>
  __forceinline T clamp(const T& value, const T& minValue, const T& maxValue)
  {
    return min(max(value, minValue), maxValue);
  }

  void* alignedMalloc(size_t size, size_t alignment);
  void alignedFree(void* ptr);

#if defined(__APPLE__)
  template<typename T>
  bool getSysctl(const char* name, T& value)
  {
    int64_t result = 0;
    size_t size = sizeof(result);

    if (sysctlbyname(name, &result, &size, nullptr, 0) != 0)
      return false;

    value = T(result);
    return true;
  }
#endif

} // namespace oidn

