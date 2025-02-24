// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#ifndef Build_hpp
#define Build_hpp

// Define MEMORY_SANITIZER to 1 if the project was build with the memory
// sanitizer enabled (-fsanitize=memory).
#if defined(__clang__) && __has_feature(memory_sanitizer)
#	define MEMORY_SANITIZER 1
#else
#	define MEMORY_SANITIZER 0
#endif

#endif  // Build_hpp
