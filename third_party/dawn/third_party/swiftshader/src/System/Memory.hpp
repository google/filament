// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#ifndef Memory_hpp
#define Memory_hpp

#include <stddef.h>
#include <stdint.h>

namespace sw {

size_t memoryPageSize();

void *allocate(size_t bytes, size_t alignment = 16);              // Never initialized.
void *allocateZeroOrPoison(size_t bytes, size_t alignment = 16);  // Initialized to zero, except in MemorySanitizer builds.

void freeMemory(void *memory);

void clear(uint16_t *memory, uint16_t element, size_t count);
void clear(uint32_t *memory, uint32_t element, size_t count);

}  // namespace sw

#endif  // Memory_hpp
