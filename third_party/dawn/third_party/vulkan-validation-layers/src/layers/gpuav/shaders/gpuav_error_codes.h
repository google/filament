// Copyright (c) 2021-2025 The Khronos Group Inc.
// Copyright (c) 2021-2025 Valve Corporation
// Copyright (c) 2021-2025 LunarG, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// Values used between the GLSL shaders and the GPU-AV logic

// NOTE: This header is included by the instrumentation shaders and glslang doesn't support #pragma once
#ifndef GPU_ERROR_CODES_H
#define GPU_ERROR_CODES_H

#ifdef __cplusplus
namespace gpuav {
namespace glsl {
#endif

// Error Groups
//
// These will match one-for-one with the file found in gpu_shader folder
const int kErrorGroupInstDescriptorIndexingOOB = 1;
const int kErrorGroupInstBufferDeviceAddress = 2;
const int kErrorGroupInstRayQuery = 3;
const int kErrorGroupGpuPreDraw = 4;
const int kErrorGroupGpuPreDispatch = 5;
const int kErrorGroupGpuPreTraceRays = 6;
const int kErrorGroupGpuCopyBufferToImage = 7;
const int kErrorGroupInstDescriptorClass = 8;
const int kErrorGroupInstIndexedDraw = 9;
const int kErrorGroupInst_Reserved_6452 = 10;  // Saved for future extension MR 6452

// Used for MultiEntry and there is no single stage set
const int kHeaderStageIdMultiEntryPoint = 0x7fffffff;  // same as spv::ExecutionModelMax

// Descriptor Indexing
//
const int kErrorSubCodeDescriptorIndexingBounds = 1;
const int kErrorSubCodeDescriptorIndexingUninitialized = 2;
const int kErrorSubCodeDescriptorIndexingDestroyed = 3;

// Descriptor Class specific errors
//
// Buffers
const int kErrorSubCodeDescriptorClassGeneralBufferBounds = 1;
// Texel Buffers
const int kErrorSubCodeDescriptorClassTexelBufferBounds = 2;

// Buffer Device Address
//
const int kErrorSubCodeBufferDeviceAddressUnallocRef = 1;
const int kErrorSubCodeBufferDeviceAddressAlignment = 2;

// Ray Query
//
const int kErrorSubCodeRayQueryNegativeMin = 1;
const int kErrorSubCodeRayQueryNegativeMax = 2;
const int kErrorSubCodeRayQueryBothSkip = 3;
const int kErrorSubCodeRayQuerySkipCull = 4;
const int kErrorSubCodeRayQueryOpaque = 5;
const int kErrorSubCodeRayQueryMinMax = 6;
const int kErrorSubCodeRayQueryMinNaN = 7;
const int kErrorSubCodeRayQueryMaxNaN = 8;
const int kErrorSubCodeRayQueryOriginNaN = 9;
const int kErrorSubCodeRayQueryDirectionNaN = 10;
const int kErrorSubCodeRayQueryOriginFinite = 11;
const int kErrorSubCodeRayQueryDirectionFinite = 12;

// Indexed Draw
//
const int kErrorSubCode_IndexedDraw_OOBVertexIndex = 1;
const int kErrorSubCode_IndexedDraw_OOBInstanceIndex = 2;

// Pre Draw
//
// The draw count exceeded the draw buffer size
const int kErrorSubCodePreDraw_DrawBufferSize = 1;
// The draw count exceeded the maxDrawCount parameter to the command
const int kErrorSubCodePreDraw_DrawCountLimit = 2;
// A firstInstance field was non-zero
const int kErrorSubCodePreDrawFirstInstance = 3;
// Mesh limit checks
const int kErrorSubCodePreDrawGroupCountX = 4;
const int kErrorSubCodePreDrawGroupCountY = 5;
const int kErrorSubCodePreDrawGroupCountZ = 6;
const int kErrorSubCodePreDrawGroupCountTotal = 7;
// The index count exceeded the index buffer size
const int kErrorSubCode_OobIndexBuffer = 8;
// An index in the index buffer exceeded the vertex buffer size
const int kErrorSubCode_OobVertexBuffer = 9;

// Pre Dispatch
//
const int kErrorSubCodePreDispatchCountLimitX = 1;
const int kErrorSubCodePreDispatchCountLimitY = 2;
const int kErrorSubCodePreDispatchCountLimitZ = 3;

// Pre Tracy Rays
//
const int kErrorSubCodePreTraceRaysLimitWidth = 1;
const int kErrorSubCodePreTraceRaysLimitHeight = 2;
const int kErrorSubCodePreTraceRaysLimitDepth = 3;
const int kErrorSubCodePreTraceRaysLimitVolume = 4;
// Pre Copy Buffer To Image
//
const int kErrorSubCodePreCopyBufferToImageBufferTexel = 1;

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif
#endif
