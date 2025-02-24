// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef marl_sanitizers_h
#define marl_sanitizers_h

// Define MARL_ADDRESS_SANITIZER_ENABLED to 1 if the project was built with the
// address sanitizer enabled (-fsanitize=address).
#if defined(__SANITIZE_ADDRESS__)
#define MARL_ADDRESS_SANITIZER_ENABLED 1
#else  // defined(__SANITIZE_ADDRESS__)
#if defined(__clang__)
#if __has_feature(address_sanitizer)
#define MARL_ADDRESS_SANITIZER_ENABLED 1
#endif  // __has_feature(address_sanitizer)
#endif  // defined(__clang__)
#endif  // defined(__SANITIZE_ADDRESS__)

// MARL_ADDRESS_SANITIZER_ONLY(X) resolves to X if
// MARL_ADDRESS_SANITIZER_ENABLED is defined to a non-zero value, otherwise
// MARL_ADDRESS_SANITIZER_ONLY() is stripped by the preprocessor.
#if MARL_ADDRESS_SANITIZER_ENABLED
#define MARL_ADDRESS_SANITIZER_ONLY(x) x
#else
#define MARL_ADDRESS_SANITIZER_ONLY(x)
#endif  // MARL_ADDRESS_SANITIZER_ENABLED

// Define MARL_MEMORY_SANITIZER_ENABLED to 1 if the project was built with the
// memory sanitizer enabled (-fsanitize=memory).
#if defined(__SANITIZE_MEMORY__)
#define MARL_MEMORY_SANITIZER_ENABLED 1
#else  // defined(__SANITIZE_MEMORY__)
#if defined(__clang__)
#if __has_feature(memory_sanitizer)
#define MARL_MEMORY_SANITIZER_ENABLED 1
#endif  // __has_feature(memory_sanitizer)
#endif  // defined(__clang__)
#endif  // defined(__SANITIZE_MEMORY__)

// MARL_MEMORY_SANITIZER_ONLY(X) resolves to X if MARL_MEMORY_SANITIZER_ENABLED
// is defined to a non-zero value, otherwise MARL_MEMORY_SANITIZER_ONLY() is
// stripped by the preprocessor.
#if MARL_MEMORY_SANITIZER_ENABLED
#define MARL_MEMORY_SANITIZER_ONLY(x) x
#else
#define MARL_MEMORY_SANITIZER_ONLY(x)
#endif  // MARL_MEMORY_SANITIZER_ENABLED

// Define MARL_THREAD_SANITIZER_ENABLED to 1 if the project was built with the
// thread sanitizer enabled (-fsanitize=thread).
#if defined(__SANITIZE_THREAD__)
#define MARL_THREAD_SANITIZER_ENABLED 1
#else  // defined(__SANITIZE_THREAD__)
#if defined(__clang__)
#if __has_feature(thread_sanitizer)
#define MARL_THREAD_SANITIZER_ENABLED 1
#endif  // __has_feature(thread_sanitizer)
#endif  // defined(__clang__)
#endif  // defined(__SANITIZE_THREAD__)

// MARL_THREAD_SANITIZER_ONLY(X) resolves to X if MARL_THREAD_SANITIZER_ENABLED
// is defined to a non-zero value, otherwise MARL_THREAD_SANITIZER_ONLY() is
// stripped by the preprocessor.
#if MARL_THREAD_SANITIZER_ENABLED
#define MARL_THREAD_SANITIZER_ONLY(x) x
#else
#define MARL_THREAD_SANITIZER_ONLY(x)
#endif  // MARL_THREAD_SANITIZER_ENABLED

// Define MARL_UNDEFINED_SANITIZER_ENABLED to 1 if the project was built with
// the undefined sanitizer enabled (-fsanitize=undefined).
#if defined(__clang__)
#if __has_feature(undefined_behavior_sanitizer)
#define MARL_UNDEFINED_SANITIZER_ENABLED 1
#endif  // __has_feature(undefined_behavior_sanitizer)
#endif  // defined(__clang__)

// MARL_UNDEFINED_SANITIZER_ONLY(X) resolves to X if
// MARL_UNDEFINED_SANITIZER_ENABLED is defined to a non-zero value, otherwise
// MARL_UNDEFINED_SANITIZER_ONLY() is stripped by the preprocessor.
#if MARL_UNDEFINED_SANITIZER_ENABLED
#define MARL_UNDEFINED_SANITIZER_ONLY(x) x
#else
#define MARL_UNDEFINED_SANITIZER_ONLY(x)
#endif  // MARL_UNDEFINED_SANITIZER_ENABLED

#endif  // marl_sanitizers_h
