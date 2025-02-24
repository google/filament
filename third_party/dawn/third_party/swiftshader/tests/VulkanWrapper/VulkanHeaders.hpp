// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BENCHMARKS_VULKAN_HEADERS_HPP_
#define BENCHMARKS_VULKAN_HEADERS_HPP_

#if !defined(USE_HEADLESS_SURFACE)
#	define USE_HEADLESS_SURFACE 0
#endif

#if !defined(_WIN32)
// @TODO: implement native Window support for current platform. For now, always use HeadlessSurface.
#	undef USE_HEADLESS_SURFACE
#	define USE_HEADLESS_SURFACE 1
#endif

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#endif
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_NODISCARD_WARNINGS
#include <vulkan/vulkan.hpp>

#endif  // BENCHMARKS_VULKAN_HEADERS_HPP_
