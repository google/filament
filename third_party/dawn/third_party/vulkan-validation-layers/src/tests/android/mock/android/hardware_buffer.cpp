/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hardware_buffer.h"

// Because test and layer have 2 instance of this file, store everything in the AHB
struct AHardwareBuffer {
    AHardwareBuffer_Desc desc;
};

// Just keep a single global AHB
static AHardwareBuffer kAHB;
static AHardwareBuffer* kpAHB = &kAHB;

int AHardwareBuffer_allocate(const AHardwareBuffer_Desc* desc, AHardwareBuffer** outBuffer) {
    *outBuffer = kpAHB;
    kAHB.desc = *desc;
    return 0;
}

void AHardwareBuffer_release(AHardwareBuffer* buffer) { (void)buffer; }

void AHardwareBuffer_describe(const AHardwareBuffer* buffer, AHardwareBuffer_Desc* outDesc) { *outDesc = buffer->desc; };