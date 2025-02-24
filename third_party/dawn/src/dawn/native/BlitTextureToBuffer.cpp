// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/BlitTextureToBuffer.h"

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/ComputePassEncoder.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/Queue.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

namespace {

constexpr uint32_t kWorkgroupSizeX = 8;
constexpr uint32_t kWorkgroupSizeY = 8;

constexpr std::string_view kDstBufferU32 = R"(
@group(0) @binding(1) var<storage, read_write> dst_buf : array<u32>;
)";

// For DepthFloat32 we can directly use f32 for the buffer array data type as we don't need packing.
constexpr std::string_view kDstBufferF32 = R"(
@group(0) @binding(1) var<storage, read_write> dst_buf : array<f32>;
)";

constexpr std::string_view kFloatTexture1D = R"(
fn textureLoadGeneral(tex: texture_1d<f32>, coords: vec3u, level: u32) -> vec4<f32> {
    return textureLoad(tex, coords.x, level);
}
@group(0) @binding(0) var src_tex : texture_1d<f32>;
)";

constexpr std::string_view kFloatTexture2D = R"(
fn textureLoadGeneral(tex: texture_2d<f32>, coords: vec3u, level: u32) -> vec4<f32> {
    return textureLoad(tex, coords.xy, level);
}
@group(0) @binding(0) var src_tex : texture_2d<f32>;
)";

constexpr std::string_view kFloatTexture2DArray = R"(
fn textureLoadGeneral(tex: texture_2d_array<f32>, coords: vec3u, level: u32) -> vec4<f32> {
    return textureLoad(tex, coords.xy, coords.z, level);
}
@group(0) @binding(0) var src_tex : texture_2d_array<f32>;
)";

constexpr std::string_view kFloatTexture3D = R"(
fn textureLoadGeneral(tex: texture_3d<f32>, coords: vec3u, level: u32) -> vec4<f32> {
    return textureLoad(tex, coords, level);
}
@group(0) @binding(0) var src_tex : texture_3d<f32>;
)";

// Cube map reference: https://en.wikipedia.org/wiki/Cube_mapping
// Function converting texel coord to sample st coord for cube texture.
constexpr std::string_view kCubeCoordCommon = R"(
fn coordToCubeSampleST(coords: vec3u, size: vec3u) -> vec3<f32> {
    var st = (vec2f(coords.xy) + vec2f(0.5, 0.5)) / vec2f(params.levelSize.xy);
    st.y = 1. - st.y;
    st = st * 2. - 1.;
    var sample_coords: vec3f;
    switch(coords.z) {
        case 0: { sample_coords = vec3f(1., st.y, -st.x); } // Positive X
        case 1: { sample_coords = vec3f(-1., st.y, st.x); } // Negative X
        case 2: { sample_coords = vec3f(st.x, 1., -st.y); } // Positive Y
        case 3: { sample_coords = vec3f(st.x, -1., st.y); } // Negative Y
        case 4: { sample_coords = vec3f(st.x, st.y, 1.); }  // Positive Z
        case 5: { sample_coords = vec3f(-st.x, st.y, -1.);} // Negative Z
        default: { return vec3f(0.); } // Unreachable
    }
    return sample_coords;
}
)";

constexpr std::string_view kFloatTextureCube = R"(
@group(1) @binding(0) var default_sampler: sampler;
fn textureLoadGeneral(tex: texture_cube<f32>, coords: vec3u, level: u32) -> vec4<f32> {
    let sample_coords = coordToCubeSampleST(coords, params.levelSize);
    return textureSampleLevel(tex, default_sampler, sample_coords, f32(level));
}
@group(0) @binding(0) var src_tex : texture_cube<f32>;
)";

constexpr std::string_view kUintTexture = R"(
fn textureLoadGeneral(tex: texture_2d<u32>, coords: vec3u, level: u32) -> vec4<u32> {
    return textureLoad(tex, coords.xy, level);
}
@group(0) @binding(0) var src_tex : texture_2d<u32>;
)";

constexpr std::string_view kUintTextureArray = R"(
fn textureLoadGeneral(tex: texture_2d_array<u32>, coords: vec3u, level: u32) -> vec4<u32> {
    return textureLoad(tex, coords.xy, coords.z, level);
}
@group(0) @binding(0) var src_tex : texture_2d_array<u32>;
)";

// textureSampleLevel doesn't support texture_cube<u32>
// Use textureGather as a workaround.
// Always choose the texel with the smallest coord (stored in w component).
// Since this is only used for Stencil8 (1 channel), we only care component idx == 0.
constexpr std::string_view kUintTextureCube = R"(
@group(1) @binding(0) var default_sampler: sampler;
fn textureLoadGeneral(tex: texture_cube<u32>, coords: vec3u, level: u32) -> vec4<u32> {
    let sample_coords = coordToCubeSampleST(coords, params.levelSize);
    return vec4<u32>(textureGather(0, tex, default_sampler, sample_coords).w);
}
@group(0) @binding(0) var src_tex : texture_cube<u32>;
)";

constexpr std::string_view kEncodeRGBA8UnormInU32 = R"(
fn encodeVectorInU32General(v: vec4f) -> u32 {
    return pack4x8unorm(v);
}
)";

constexpr std::string_view kEncodeRGBA8SnormInU32 = R"(
fn encodeVectorInU32General(v: vec4f) -> u32 {
    return pack4x8snorm(v);
}
)";

// Storing and swizzling bgra8unorm texel values and convert to u32.
constexpr std::string_view kEncodeBGRA8UnormInU32 = R"(
fn encodeVectorInU32General(v: vec4f) -> u32 {
    return pack4x8unorm(v.bgra);
}
)";

constexpr std::string_view kEncodeRG16FloatInU32 = R"(
fn encodeVectorInU32General(v: vec2f) -> u32 {
    return pack2x16float(v);
}
)";

// Each thread is responsible for reading (packTexelCount) texel and packing them into a 4-byte u32.
constexpr std::string_view kCommonHead = R"(
struct Params {
    // copyExtent
    srcOrigin: vec3u,
    // How many texel values one thread needs to pack (1, 2, or 4)
    packTexelCount: u32,
    srcExtent: vec3u,
    mipLevel: u32,
    // GPUImageDataLayout
    bytesPerRow: u32,
    rowsPerImage: u32,
    offset: u32,
    shift: u32,
    // Used for cube sample
    levelSize: vec3u,
    pad0: u32,
    texelSize: u32,
    numU32PerRowNeedsWriting: u32,
    readPreviousRow: u32,
    isCompactImage: u32,
};

@group(0) @binding(2) var<uniform> params : Params;

override workgroupSizeX: u32;
override workgroupSizeY: u32;

// Size of one element in the destination buffer this thread will write to.
override gOutputUnitSize: u32;

@compute @workgroup_size(workgroupSizeX, workgroupSizeY, 1) fn main
(@builtin(global_invocation_id) id : vec3u) {
)";

constexpr std::string_view kCommonStart = R"(
let srcBoundary = params.srcOrigin + params.srcExtent;
let coord0 = vec3u(id.x * params.packTexelCount, id.y, id.z) + params.srcOrigin;
if (any(coord0 >= srcBoundary)) {
    return;
}

let indicesPerRow = params.bytesPerRow / gOutputUnitSize;
let indicesOffset = params.offset / gOutputUnitSize;
let dstOffset = indicesOffset + id.x + id.y * indicesPerRow + id.z * indicesPerRow * params.rowsPerImage;
)";

constexpr std::string_view kCommonEnd = R"(
    dst_buf[dstOffset] = result;
}
)";

constexpr std::string_view kPackStencil8ToU32 = R"(
    // Storing stencil8 texel values
    var result: u32 = 0xff & textureLoadGeneral(src_tex, coord0, params.mipLevel).r;

    if (coord0.x + 4u <= srcBoundary.x) {
        // All 4 texels for this thread are within texture bounds.
        for (var i = 1u; i < 4u; i += 1u) {
            let coordi = coord0 + vec3u(i, 0, 0);
            let ri = 0xff & textureLoadGeneral(src_tex, coordi, params.mipLevel).r;
            result |= ri << (i * 8u);
        }
    } else {
        // Otherwise, srcExtent.x is not a multiple of 4 and this thread is at right edge of the texture
        // To preserve the original buffer content, we need to read from the buffer and pack it together with other values.
        let original: u32 = dst_buf[dstOffset];
        result |= original & 0xffffff00;

        for (var i = 1u; i < 4u; i += 1u) {
            let coordi = coord0 + vec3u(i, 0, 0);
            if (coordi.x >= srcBoundary.x) {
                break;
            }
            let ri = 0xff & textureLoadGeneral(src_tex, coordi, params.mipLevel).r;
            result |= ri << (i * 8u);
        }
    }
)";

// Color format R8Snorm and RG8Snorm T2B copy doesn't require offset to be multiple of 4 bytes,
// making it more complicated than other formats.
// TODO(dawn:1886): potentially separate "middle of the image" case
// and "on the edge" case into different shaders and passes for better performance.
constexpr std::string_view kNonMultipleOf4OffsetStart = R"(
let readPreviousRow: bool = params.readPreviousRow == 1;
let isCompactImage: bool = params.isCompactImage == 1;
let idBoundary = vec3u(params.numU32PerRowNeedsWriting
    - select(1u, 0u,
        params.shift == 0 ||
        // one more thread at end of row
        !readPreviousRow ||
        // one more thread at end of image
        (!isCompactImage && id.y == params.srcExtent.y - 1) ||
        // one more thread at end of buffer
        (id.y == params.srcExtent.y - 1 && id.z == params.srcExtent.z - 1)
        )
    , params.srcExtent.y, params.srcExtent.z);
if (any(id >= idBoundary)) {
    return;
}

let byteOffset = params.offset + id.x * gOutputUnitSize
    + id.y * params.bytesPerRow
    + id.z * params.bytesPerRow * params.rowsPerImage;
let dstOffset = byteOffset / gOutputUnitSize;
let srcBoundary = params.srcOrigin + params.srcExtent;

// Start coord, End coord
var coordS = vec3u(id.x * params.packTexelCount, id.y, id.z) + params.srcOrigin;
var coordE = coordS;
coordE.x += params.packTexelCount - 1;

if (params.shift > 0) {
    // Adjust coordS
    if (id.x == 0) {
        // Front of a row
        if (readPreviousRow) {
            // Needs reading from previous row
            coordS.x += params.bytesPerRow / params.texelSize - params.shift;
            if (id.y == 0) {
                // Front of a layer
                if (isCompactImage) {
                    // Needs reading from previous layer
                    coordS.y += params.srcExtent.y - 1;
                    if (id.z > 0) {
                        coordS.z -= 1;
                    }
                }
            } else {
                coordS.y -= 1;
            }
        }
    } else {
        coordS.x -= params.shift;
    }
    coordE.x -= params.shift;
}

let readDstBufAtStart: bool = params.shift > 0 && (
        all(id == vec3u(0u))    // start of buffer
        || (id.x == 0 && (!readPreviousRow      // start of non-compact row
            || (id.y == 0 && !isCompactImage)   // start of non-compact image
        )));
let readDstBufAtEnd: bool = coordE.x >= srcBoundary.x;
)";

// R8snorm: texelByte = 1; each thread reads 1 ~ 4 texels.
// Different scenarios are listed below:
//
// * In the middle of the row: reads 4 texels
//       |  x  | x+1 | x+2 | x+3 |
//
// * At the edge of the row: when offset % 4 > 0
//   - when copyWidth % bytesPerRow == 0 (compact row), read 4 texels
//       e.g. offset = 1; copyWidth = 256;
//       | 255,y-1 | 0,y | 1,y | 2,y |
//   - when copyWidth % bytesPerRow > 0 || rowsPerImage > copyHeight (sparse row / sparse image)
//     One more thread is added to the end of each row,
//     reads 1 ~ 3 texels, reads dst buf values
//       e.g. offset = 1; copyWidth = 128; mask = 0xffffff00;
//       | 127,y-1 |  b  |  b  |  b  |
//   - when copyWidth % bytesPerRow > 0 && copyWidth + offset % 4 > bytesPerRow (special case)
//     reads 1 ~ 3 texels, reads dst buf values; mask = 0x0000ff00;
//       e.g. offset = 1; copyWidth = 255;
//       | 254,y-1 |  b  | 0,y | 1,y |
//
// * At the start of the whole copy:
//   - when offset % 4 == 0, reads 4 texels
//   - when offset % 4 > 0, reads 1 ~ 3 texels, reads dst buf values
//       e.g. offset = 1; mask = 0x000000ff;
//       |  b  |  0  |  1  |  2  |
//       e.g. offset = 1, copyWidth = 2; mask = 0xff0000ff;
//       |  b  |  0  |  1  |  b  |
//
// * At the end of the whole copy:
//   - reads 1 ~ 4 texels, reads dst buf values;
//       e.g. offset = 0; copyWidth = 256;
//       | 252 | 253 | 254 | 255 |
//       e.g. offset = 1; copyWidth = 256; mask = 0xffffff00;
//       | 255 |  b  |  b  |  b  |

constexpr std::string_view kPackR8ToU32 = R"(
// Result bits to store into dst_buf
var result: u32 = 0u;
// Storing xnorm8 texel values
// later called by pack4x8xnorm to convert to u32.
var v: vec4<f32>;

// dstBuf value is used for starting part.
var mask: u32 = 0xffffffffu;
if (!readDstBufAtStart) {
    // coordS is used
    mask &= 0xffffff00u;
    v[0] = textureLoadGeneral(src_tex, coordS, params.mipLevel).r;
} else {
    // start of buffer, boundary check
    if (coordE.x >= 1) {
        if (coordE.x - 1 < srcBoundary.x) {
            mask &= 0xff00ffffu;
            v[2] = textureLoadGeneral(src_tex, coordE - vec3u(1, 0, 0), params.mipLevel).r;
        }

        if (coordE.x >= 2) {
            if (coordE.x - 2 < srcBoundary.x) {
                mask &= 0xffff00ffu;
                v[1] = textureLoadGeneral(src_tex, coordE - vec3u(2, 0, 0), params.mipLevel).r;
            }

            if (coordE.x >= 3) {
                if (coordE.x - 3 < srcBoundary.x) {
                    mask &= 0xffffff00u;
                    v[0] = textureLoadGeneral(src_tex, coordE - vec3u(3, 0, 0), params.mipLevel).r;
                }
            }
        }
    }
}

if (coordE.x < srcBoundary.x) {
    mask &= 0x00ffffffu;
    v[3] = textureLoadGeneral(src_tex, coordE, params.mipLevel).r;
} else {
    // coordE is not used
    // dstBuf value is used for later part.
    // end of buffer (last thread) / end of non-compact row + x boundary check
    if (coordE.x - 2 < srcBoundary.x) {
        mask &= 0xffff00ffu;
        v[1] = textureLoadGeneral(src_tex, coordE - vec3u(2, 0, 0), params.mipLevel).r;
        if (coordE.x - 1 < srcBoundary.x) {
            mask &= 0xff00ffffu;
            v[2] = textureLoadGeneral(src_tex, coordE - vec3u(1, 0, 0), params.mipLevel).r;
        }
    }
}

if (readDstBufAtStart || readDstBufAtEnd) {
    let original: u32 = dst_buf[dstOffset];
    result = (original & mask) | (encodeVectorInU32General(v) & ~mask);
} else {
    var coord1: vec3u;
    var coord2: vec3u;
    if (coordS.x < coordE.x) {
        // middle of row
        coord1 = coordE - vec3u(2, 0, 0);
        coord2 = coordE - vec3u(1, 0, 0);
    } else {
        // start of row
        switch params.shift {
            case 0: {
                coord1 = coordS + vec3u(1, 0, 0);
                coord2 = coordS + vec3u(2, 0, 0);
            }
            case 1: {
                coord1 = coordE - vec3u(2, 0, 0);
                coord2 = coordE - vec3u(1, 0, 0);
            }
            case 2: {
                coord1 = coordS + vec3u(1, 0, 0);
                coord2 = coordE - vec3u(1, 0, 0);
            }
            case 3: {
                coord1 = coordS + vec3u(1, 0, 0);
                coord2 = coordS + vec3u(2, 0, 0);
            }
            default: {
                return; // unreachable when shift == 0
            }
        }
    }

    if (coord1.x < srcBoundary.x) {
        mask &= 0xffff00ffu;
        v[1] = textureLoadGeneral(src_tex, coord1, params.mipLevel).r;
    }
    if (coord2.x < srcBoundary.x) {
        mask &= 0xff00ffffu;
        v[2] = textureLoadGeneral(src_tex, coord2, params.mipLevel).r;
    }

    let readDstBufAtMid: bool = (params.srcExtent.x + params.shift > params.bytesPerRow)
        && (params.srcExtent.x < params.bytesPerRow);
    if (readDstBufAtMid && id.x == 0) {
        let original: u32 = dst_buf[dstOffset];
        result = (original & mask) | (encodeVectorInU32General(v) & ~mask);
    } else {
        result = encodeVectorInU32General(v);
    }
}
)";

// RG8snorm: texelByte = 2; each thread reads 1 ~ 2 texels.
// Different scenarios are listed below:
//
// * In the middle of the row: reads 2 texels
//       |    x    |   x+1   |
//
// * At the edge of the row: when offset % 4 > 0
//   - when copyWidth % bytesPerRow == 0 (compact row), read 2 texels
//       e.g. offset = 2; copyWidth = 128;
//       | 127,y-1 |   0,y   |
//   - when copyWidth % bytesPerRow > 0 || rowsPerImage > copyHeight (sparse row / sparse image)
//     One more thread is added to the end of each row,
//     reads 1 texels, reads dst buf values
//       e.g. offset = 1; copyWidth = 64; mask = 0xffff0000;
//       |  63,y-1 |    b    |
//
// * At the start of the whole copy:
//   - when offset % 4 == 0, reads 2 texels
//   - when offset % 4 > 0, reads 1 texels, reads dst buf values
//       e.g. offset = 2; mask = 0x0000ffff;
//       |    b    |    0    |
//
// * At the end of the whole copy:
//   - reads 1 ~ 2 texels, reads dst buf values;
//       e.g. offset = 0; copyWidth = 128;
//       |   126   |   127   |
//       e.g. offset = 1; copyWidth = 128; mask = 0xffff0000;
//       |   127   |    b    |

constexpr std::string_view kPackRG8ToU32 = R"(
// Result bits to store into dst_buf
var result: u32 = 0u;
// Storing snorm8 texel values
// later called by pack4x8xnorm to convert to u32.
var v: vec4<f32>;

// dstBuf value is used for starting part.
var mask: u32 = 0xffffffffu;
if (!readDstBufAtStart) {
    // coordS is used
    mask &= 0xffff0000u;
    let texel0 = textureLoadGeneral(src_tex, coordS, params.mipLevel).rg;
    v[0] = texel0.r;
    v[1] = texel0.g;
}

if (coordE.x < srcBoundary.x) {
    // coordE is used
    mask &= 0x0000ffffu;
    let texel1 = textureLoadGeneral(src_tex, coordE, params.mipLevel).rg;
    v[2] = texel1.r;
    v[3] = texel1.g;
}

if (readDstBufAtStart || readDstBufAtEnd) {
    let original: u32 = dst_buf[dstOffset];
    result = (original & mask) | (encodeVectorInU32General(v) & ~mask);
} else {
    result = encodeVectorInU32General(v);
}
)";

// R16: texelByte = 2; each thread reads 1 ~ 2 texels.
// General packing algorithm is similar to kPackRG8ToU32.
constexpr std::string_view kPackR16ToU32 = R"(
// Result bits to store into dst_buf
var result: u32 = 0u;
// Storing half texel values
// later called by pack2x16unorm to convert to u32.
var v: vec2f;

// dstBuf value is used for starting part.
var mask: u32 = 0xffffffffu;
if (!readDstBufAtStart) {
    // coordS is used
    mask &= 0xffff0000u;
    let texel0 = textureLoadGeneral(src_tex, coordS, params.mipLevel).r;
    v[0] = texel0;
}

if (coordE.x < srcBoundary.x) {
    // coordE is used
    mask &= 0x0000ffffu;
    let texel1 = textureLoadGeneral(src_tex, coordE, params.mipLevel).r;
    v[1] = texel1;
}

if (readDstBufAtStart || readDstBufAtEnd) {
    let original: u32 = dst_buf[dstOffset];
    result = (original & mask) | (encodeVectorInU32General(v) & ~mask);
} else {
    result = encodeVectorInU32General(v);
}
)";

constexpr std::string_view kPackRG16ToU32 = R"(
    let v: vec2f = textureLoadGeneral(src_tex, coord0, params.mipLevel).rg;
    let result = encodeVectorInU32General(v);
)";

// Load RGBA16 and pack to 2 uint4_t
constexpr std::string_view kLoadRGBA16ToU32 = R"(
    let v: vec4f = textureLoadGeneral(src_tex, coord0, params.mipLevel);
    // dstOffset is based on 8 bytes so we need to multiply by 2 to get uint32 offset.
    let uintOffset = dstOffset << 1;
    dst_buf[uintOffset] = encodeVectorInU32General(v.rg);
    dst_buf[uintOffset + 1] = encodeVectorInU32General(v.ba);
}
)";

// ShaderF16 extension is only enabled by GL_AMD_gpu_shader_half_float for GL
// so we should not use it generally for the emulation.
// As a result we are using f32 and array<u32> to do all the math and byte manipulation.
// If we have 2-byte scalar type (f16, u16) it can be a bit easier when writing to the storage
// buffer.
constexpr std::string_view kPackDepth16UnormToU32 = R"(
    // Result bits to store into dst_buf
    var result: u32 = 0u;
    // Storing depth16unorm texel values
    // later called by pack2x16unorm to convert to u32.
    var v: vec2<f32>;
    v[0] = textureLoadGeneral(src_tex, coord0, params.mipLevel).r;

    let coord1 = coord0 + vec3u(1, 0, 0);
    if (coord1.x < srcBoundary.x) {
        // Make sure coord1 is still within the copy boundary.
        v[1] = textureLoadGeneral(src_tex, coord1, params.mipLevel).r;
        result = pack2x16unorm(v);
    } else {
        // Otherwise, srcExtent.x is not a multiple of 2 and this thread is at right edge of the texture
        // To preserve the original buffer content, we need to read from the buffer and pack it together with other values.
        // TODO(dawn:1782): profiling against making a separate pass for this edge case
        // as it requires reading from dst_buf.
        let original: u32 = dst_buf[dstOffset];
        const mask = 0xffff0000u;
        result = (original & mask) | (pack2x16unorm(v) & ~mask);
    }
)";

// Storing rgba texel values
// later called by encodeVectorInU32General to convert to u32.
constexpr std::string_view kPackRGBAToU32 = R"(
    let v = textureLoadGeneral(src_tex, coord0, params.mipLevel);
    let result: u32 = encodeVectorInU32General(v);
)";

// Storing rgb9e5ufloat texel values
// In this format float is represented as
// 2^(exponent - bias) * (mantissa / 2^numMantissaBits)
// Packing algorithm is from:
// https://registry.khronos.org/OpenGL/extensions/EXT/EXT_texture_shared_exponent.txt
//
// Note: there are multiple bytes that could represent the same value in this format.
// e.g.
// 0x0a090807 and 0x0412100e both unpack to
// [8.344650268554688e-7, 0.000015735626220703125, 0.000015497207641601562]
// So the bytes copied via blit could be different.
constexpr std::string_view kEncodeRGB9E5UfloatInU32 = R"(
fn encodeVectorInU32General(v: vec4f) -> u32 {
    const n = 9; // number of mantissa bits
    const e_max = 31; // max exponent
    const b = 15; // exponent bias
    const sharedexp_max: f32 = (f32((1 << n) - 1) / f32(1 << n)) * (1 << (e_max - b));

    let red_c = clamp(v.r, 0.0, sharedexp_max);
    let green_c = clamp(v.g, 0.0, sharedexp_max);
    let blue_c = clamp(v.b, 0.0, sharedexp_max);

    let max_c = max(max(red_c, green_c), blue_c);
    let exp_shared_p: i32 = max(-b - 1, i32(floor(log2(max_c)))) + 1 + b;
    let max_s = u32(floor(max_c / exp2(f32(exp_shared_p - b - n)) + 0.5));
    var exp_shared = exp_shared_p;
    if (max_s == (1 << n)) {
        exp_shared += 1;
    }

    let scalar = 1.0 / exp2(f32(exp_shared - b - n));
    let red_s = u32(red_c * scalar + 0.5);
    let green_s = u32(green_c * scalar + 0.5);
    let blue_s = u32(blue_c * scalar + 0.5);

    const mask_9 = 0x1ffu;
    let result = (u32(exp_shared) << 27u) |
        ((blue_s & mask_9) << 18u) |
        ((green_s & mask_9) << 9u) |
        (red_s & mask_9);

    return result;
}
)";

// Storing rg11b10ufloat texel values
// Reference:
// https://www.khronos.org/opengl/wiki/Small_Float_Formats
constexpr std::string_view kEncodeRG11B10UfloatInU32 = R"(
fn encodeVectorInU32General(v: vec4f) -> u32 {
    const n_rg = 6;    // number of mantissa bits (RG)
    const n_b = 5;    // number of mantissa bits (B)
    const e_max = 31;   // max exponent
    const b = 15;    // exponent bias

    // Calculate the exponent (biased)
    let rbe = select(i32(floor(log2(v.r))), -b, v.r == 0.0);
    let gbe = select(i32(floor(log2(v.g))), -b, v.g == 0.0);
    let bbe = select(i32(floor(log2(v.b))), -b, v.b == 0.0);

    // Calculate the exponent bits value.
    let re = clamp(rbe + b, 0, e_max);
    let ge = clamp(gbe + b, 0, e_max);
    let be = clamp(bbe + b, 0, e_max);

    // Calculate the mantissa for each component.
    let rm = u32(round( select(v.r * exp2(-f32(re - b)) - 1.0, v.r * exp2(f32(b-1)), re == 0) * f32(1 << n_rg) ));
    let gm = u32(round( select(v.g * exp2(-f32(ge - b)) - 1.0, v.g * exp2(f32(b-1)), ge == 0) * f32(1 << n_rg) ));
    let bm = u32(round( select(v.b * exp2(-f32(be - b)) - 1.0, v.b * exp2(f32(b-1)), be == 0) * f32(1 << n_b) ));

    let red = u32(re << n_rg) | rm;
    let green = u32(ge << n_rg) | gm;
    let blue = u32(be << n_b) | bm;

    return (blue << 22) | (green << 11) | red;
}
)";

// Directly loading float32 values into dst_buf
// No bit manipulation and packing is needed.
constexpr std::string_view kLoadR32Float = R"(
    dst_buf[dstOffset] = textureLoadGeneral(src_tex, coord0, params.mipLevel).r;
}
)";
constexpr std::string_view kLoadRG32Float = R"(
    let v = textureLoadGeneral(src_tex, coord0, params.mipLevel);
    // dstOffset is based on 8 bytes so we need to multiply by 2 to get uint32 offset.
    let uintOffset = dstOffset << 1;
    dst_buf[uintOffset] = v.r;
    dst_buf[uintOffset + 1u] = v.g;
}
)";
constexpr std::string_view kLoadRGBA32Float = R"(
    let v = textureLoadGeneral(src_tex, coord0, params.mipLevel);
    // dstOffset is based on 16 bytes so we need to multiply by 4.
    let uintOffset = dstOffset << 2;
    dst_buf[uintOffset] = v.r;
    dst_buf[uintOffset + 1u] = v.g;
    dst_buf[uintOffset + 2u] = v.b;
    dst_buf[uintOffset + 3u] = v.a;
}
)";

ResultOrError<Ref<ComputePipelineBase>> GetOrCreateTextureToBufferPipeline(
    DeviceBase* device,
    const TextureCopy& src,
    wgpu::TextureViewDimension viewDimension) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();

    const Format& format = src.texture->GetFormat();

    auto iter = store->blitTextureToBufferComputePipelines.find({format.format, viewDimension});
    if (iter != store->blitTextureToBufferComputePipelines.end()) {
        return iter->second;
    }

    ShaderSourceWGSL wgslDesc = {};
    ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;

    wgpu::TextureSampleType textureSampleType;
    std::string shader;

    auto AppendFloatTextureHead = [&]() {
        switch (viewDimension) {
            case wgpu::TextureViewDimension::e1D:
                shader += kFloatTexture1D;
                break;
            case wgpu::TextureViewDimension::e2D:
                shader += kFloatTexture2D;
                break;
            case wgpu::TextureViewDimension::e2DArray:
                shader += kFloatTexture2DArray;
                break;
            case wgpu::TextureViewDimension::e3D:
                shader += kFloatTexture3D;
                break;
            case wgpu::TextureViewDimension::Cube:
                shader += kCubeCoordCommon;
                shader += kFloatTextureCube;
                break;
            default:
                DAWN_UNREACHABLE();
        }
    };
    auto AppendStencilTextureHead = [&]() {
        switch (viewDimension) {
            // Stencil cannot have e1D texture.
            case wgpu::TextureViewDimension::e2D:
                shader += kUintTexture;
                break;
            case wgpu::TextureViewDimension::e2DArray:
                shader += kUintTextureArray;
                break;
            case wgpu::TextureViewDimension::Cube:
                shader += kCubeCoordCommon;
                shader += kUintTextureCube;
                break;
            default:
                DAWN_UNREACHABLE();
        }
    };

    switch (format.format) {
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R8Unorm:
            AppendFloatTextureHead();
            shader += kDstBufferU32;
            shader += format.IsSnorm() ? kEncodeRGBA8SnormInU32 : kEncodeRGBA8UnormInU32;
            shader += kCommonHead;
            shader += kNonMultipleOf4OffsetStart;
            shader += kPackR8ToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RG8Unorm:
            AppendFloatTextureHead();
            shader += kDstBufferU32;
            shader += format.IsSnorm() ? kEncodeRGBA8SnormInU32 : kEncodeRGBA8UnormInU32;
            shader += kCommonHead;
            shader += kNonMultipleOf4OffsetStart;
            shader += kPackRG8ToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Unorm:
            AppendFloatTextureHead();
            shader += kDstBufferU32;
            shader += format.IsSnorm() ? kEncodeRGBA8SnormInU32 : kEncodeRGBA8UnormInU32;
            shader += kCommonHead;
            shader += kCommonStart;
            shader += kPackRGBAToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::BGRA8Unorm:
            AppendFloatTextureHead();
            shader += kDstBufferU32;
            shader += kEncodeBGRA8UnormInU32;
            shader += kCommonHead;
            shader += kCommonStart;
            shader += kPackRGBAToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::RGB9E5Ufloat:
            AppendFloatTextureHead();
            shader += kDstBufferU32;
            shader += kEncodeRGB9E5UfloatInU32;
            shader += kCommonHead;
            shader += kCommonStart;
            shader += kPackRGBAToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::RG11B10Ufloat:
            AppendFloatTextureHead();
            shader += kDstBufferU32;
            shader += kEncodeRG11B10UfloatInU32;
            shader += kCommonHead;
            shader += kCommonStart;
            shader += kPackRGBAToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::RG16Float:
            AppendFloatTextureHead();
            shader += kDstBufferU32;
            shader += kEncodeRG16FloatInU32;
            shader += kCommonHead;
            if (format.format == wgpu::TextureFormat::R16Float) {
                shader += kNonMultipleOf4OffsetStart;
                shader += kPackR16ToU32;
            } else {
                shader += kCommonStart;
                shader += kPackRG16ToU32;
            }
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::UnfilterableFloat;
            break;
        case wgpu::TextureFormat::RGBA16Float:
            AppendFloatTextureHead();
            shader += kDstBufferU32;
            shader += kEncodeRG16FloatInU32;
            shader += kCommonHead;
            shader += kCommonStart;
            shader += kLoadRGBA16ToU32;
            textureSampleType = wgpu::TextureSampleType::UnfilterableFloat;
            break;
        case wgpu::TextureFormat::Depth16Unorm:
            AppendFloatTextureHead();
            shader += kDstBufferU32;
            shader += kCommonHead;
            shader += kCommonStart;
            shader += kPackDepth16UnormToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::UnfilterableFloat;
            break;
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::R32Float:
            AppendFloatTextureHead();
            shader += kDstBufferF32;
            shader += kCommonHead;
            shader += kCommonStart;
            shader += kLoadR32Float;
            textureSampleType = wgpu::TextureSampleType::UnfilterableFloat;
            break;
        case wgpu::TextureFormat::RG32Float:
            AppendFloatTextureHead();
            shader += kDstBufferF32;
            shader += kCommonHead;
            shader += kCommonStart;
            shader += kLoadRG32Float;
            textureSampleType = wgpu::TextureSampleType::UnfilterableFloat;
            break;
        case wgpu::TextureFormat::RGBA32Float:
            AppendFloatTextureHead();
            shader += kDstBufferF32;
            shader += kCommonHead;
            shader += kCommonStart;
            shader += kLoadRGBA32Float;
            textureSampleType = wgpu::TextureSampleType::UnfilterableFloat;
            break;
        case wgpu::TextureFormat::Stencil8:
        case wgpu::TextureFormat::Depth24PlusStencil8:
            // Depth24PlusStencil8 can only copy with stencil aspect and is gated by validation.
            AppendStencilTextureHead();
            shader += kDstBufferU32;
            shader += kCommonHead;
            shader += kCommonStart;
            shader += kPackStencil8ToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Uint;
            break;
        case wgpu::TextureFormat::Depth32FloatStencil8: {
            // Depth32FloatStencil8 is not supported on OpenGL/OpenGLES where the blit path is
            // enabled by default. But could be hit if the blit path toggle is manually set on other
            // backends.
            switch (src.aspect) {
                case Aspect::Depth:
                    AppendFloatTextureHead();
                    shader += kDstBufferF32;
                    shader += kCommonHead;
                    shader += kCommonStart;
                    shader += kLoadR32Float;
                    textureSampleType = wgpu::TextureSampleType::UnfilterableFloat;
                    break;
                case Aspect::Stencil:
                    AppendStencilTextureHead();
                    shader += kDstBufferU32;
                    shader += kCommonHead;
                    shader += kCommonStart;
                    shader += kPackStencil8ToU32;
                    shader += kCommonEnd;
                    textureSampleType = wgpu::TextureSampleType::Uint;
                    break;
                default:
                    DAWN_UNREACHABLE();
            }
            break;
        }
        default:
            DAWN_UNREACHABLE();
    }

    wgslDesc.code = shader.c_str();

    Ref<ShaderModuleBase> shaderModule;
    DAWN_TRY_ASSIGN(shaderModule, device->CreateShaderModule(&shaderModuleDesc));

    Ref<BindGroupLayoutBase> bindGroupLayout0;
    DAWN_TRY_ASSIGN(bindGroupLayout0,
                    utils::MakeBindGroupLayout(
                        device,
                        {
                            {0, wgpu::ShaderStage::Compute, textureSampleType, viewDimension},
                            {1, wgpu::ShaderStage::Compute, kInternalStorageBufferBinding},
                            {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform},
                        },
                        /* allowInternalBinding */ true));

    Ref<PipelineLayoutBase> pipelineLayout;
    if (viewDimension == wgpu::TextureViewDimension::Cube) {
        // Cube texture requires an extra sampler to call textureSampleLevel
        Ref<BindGroupLayoutBase> bindGroupLayout1;
        DAWN_TRY_ASSIGN(bindGroupLayout1,
                        utils::MakeBindGroupLayout(device,
                                                   {
                                                       {0, wgpu::ShaderStage::Compute,
                                                        wgpu::SamplerBindingType::NonFiltering},
                                                   },
                                                   /* allowInternalBinding */ true));

        std::array<BindGroupLayoutBase*, 2> bindGroupLayouts = {bindGroupLayout0.Get(),
                                                                bindGroupLayout1.Get()};

        PipelineLayoutDescriptor descriptor;
        descriptor.bindGroupLayoutCount = bindGroupLayouts.size();
        descriptor.bindGroupLayouts = bindGroupLayouts.data();
        DAWN_TRY_ASSIGN(pipelineLayout, device->CreatePipelineLayout(&descriptor));
    } else {
        DAWN_TRY_ASSIGN(pipelineLayout, utils::MakeBasicPipelineLayout(device, bindGroupLayout0));
    }

    ComputePipelineDescriptor computePipelineDescriptor = {};
    computePipelineDescriptor.layout = pipelineLayout.Get();
    computePipelineDescriptor.compute.module = shaderModule.Get();
    computePipelineDescriptor.compute.entryPoint = "main";

    const uint32_t bytesPerTexel = format.GetAspectInfo(src.aspect).block.byteSize;
    // Size of one unit for a thread to write to. For format < 4 bytes, we always write 4 bytes at a
    // time.
    const uint32_t outputUnitSize = std::max(bytesPerTexel, 4u);
    const uint32_t adjustedWorkGroupSizeY =
        (viewDimension == wgpu::TextureViewDimension::e1D) ? 1 : kWorkgroupSizeY;
    const std::array<ConstantEntry, 3> constants = {{
        {nullptr, "workgroupSizeX", kWorkgroupSizeX},
        {nullptr, "workgroupSizeY", static_cast<double>(adjustedWorkGroupSizeY)},
        {nullptr, "gOutputUnitSize", static_cast<double>(outputUnitSize)},
    }};
    computePipelineDescriptor.compute.constantCount = constants.size();
    computePipelineDescriptor.compute.constants = constants.data();

    Ref<ComputePipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, device->CreateComputePipeline(&computePipelineDescriptor));
    store->blitTextureToBufferComputePipelines.emplace(std::make_pair(format.format, viewDimension),
                                                       pipeline);
    return pipeline;
}

}  // anonymous namespace

bool IsFormatSupportedByTextureToBufferBlit(wgpu::TextureFormat format) {
    // TODO(348654098): Eventually we should support all non-compressed formats. For now, just list
    // a subset of them that we support.
    switch (format) {
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::RGB9E5Ufloat:
        case wgpu::TextureFormat::RG11B10Ufloat:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RGBA32Float:
        case wgpu::TextureFormat::Depth16Unorm:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Stencil8:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

MaybeError BlitTextureToBuffer(DeviceBase* device,
                               CommandEncoder* commandEncoder,
                               const TextureCopy& src,
                               const BufferCopy& dst,
                               const Extent3D& copyExtent) {
    wgpu::TextureViewDimension textureViewDimension;
    {
        if (!device->HasFlexibleTextureViews()) {
            textureViewDimension = src.texture->GetCompatibilityTextureBindingViewDimension();
        } else {
            wgpu::TextureDimension dimension = src.texture->GetDimension();
            switch (dimension) {
                case wgpu::TextureDimension::Undefined:
                    DAWN_UNREACHABLE();
                case wgpu::TextureDimension::e1D:
                    textureViewDimension = wgpu::TextureViewDimension::e1D;
                    break;
                case wgpu::TextureDimension::e2D:
                    if (src.texture->GetArrayLayers() > 1) {
                        textureViewDimension = wgpu::TextureViewDimension::e2DArray;
                    } else {
                        textureViewDimension = wgpu::TextureViewDimension::e2D;
                    }
                    break;
                case wgpu::TextureDimension::e3D:
                    textureViewDimension = wgpu::TextureViewDimension::e3D;
                    break;
            }
        }
    }
    DAWN_ASSERT(textureViewDimension != wgpu::TextureViewDimension::Undefined &&
                textureViewDimension != wgpu::TextureViewDimension::CubeArray);

    Ref<ComputePipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline,
                    GetOrCreateTextureToBufferPipeline(device, src, textureViewDimension));

    const Format& format = src.texture->GetFormat();

    const auto& blockInfo = format.GetAspectInfo(src.aspect).block;
    const uint32_t bytesPerTexel = blockInfo.byteSize;
    uint32_t workgroupCountX = 1;
    uint32_t workgroupCountY = (textureViewDimension == wgpu::TextureViewDimension::e1D)
                                   ? 1
                                   : (copyExtent.height + kWorkgroupSizeY - 1) / kWorkgroupSizeY;
    uint32_t workgroupCountZ = copyExtent.depthOrArrayLayers;

    uint32_t numU32PerRowNeedsWriting = 0;
    const auto ssboAlignment = device->GetLimits().v1.minStorageBufferOffsetAlignment;
    // shaderStartOffset is the offset to start writing in shader.
    const uint64_t shaderStartOffset = dst.offset % ssboAlignment;
    // shaderBindingOffset is the aligned offset we will bind the buffer. Note: this value could
    // change if we use an intermediate buffer.
    uint64_t shaderBindingOffset = dst.offset - shaderStartOffset;
    bool readPreviousRow = false;
    if (bytesPerTexel < 4 && !format.HasDepthOrStencil()) {
        uint32_t extraBytes = shaderStartOffset % 4;

        // Between rows and image (whether thread at end of each row needs read start of next
        // row)
        readPreviousRow = ((copyExtent.width * bytesPerTexel) + extraBytes > dst.bytesPerRow);

        // number of u32 needs writing:
        // numU32PerRowNeedsWriting = bytesPerTexel * copyExtent.width / 4 + (1 or 0)
        // One more thread is needed when offset % 4 > 0 and the end of the buffer occupies one
        // more 4-byte word. e.g. for R8Snorm copyWidth = 256, when offset = 0, 64 u32 needs
        // writing; when offset = 1, 65 u32 needs writing; (The first u32 needs reading 3 texels
        // and mix up with the original buffer value, the last u32 needs reading 1 texel and mix
        // up with the original buffer value);
        numU32PerRowNeedsWriting = (bytesPerTexel * copyExtent.width + extraBytes + 3) / 4;
        workgroupCountX = Align(numU32PerRowNeedsWriting, kWorkgroupSizeX) / kWorkgroupSizeX;
    } else {
        switch (bytesPerTexel) {
            case 1:
                // One thread is responsible for writing four texel values (x, y) ~ (x+3, y).
                workgroupCountX =
                    Align(copyExtent.width, 4 * kWorkgroupSizeX) / (4 * kWorkgroupSizeX);
                break;
            case 2:
                // One thread is responsible for writing two texel values (x, y) and (x+1, y).
                workgroupCountX =
                    Align(copyExtent.width, 2 * kWorkgroupSizeX) / (2 * kWorkgroupSizeX);
                break;
            case 4:
            case 8:
            case 16:
                workgroupCountX = Align(copyExtent.width, kWorkgroupSizeX) / kWorkgroupSizeX;
                break;
            default:
                DAWN_UNREACHABLE();
        }
    }

    // Allow internal usages since we need to use the source as a texture binding
    // and buffer as a storage binding.
    auto scope = commandEncoder->MakeInternalUsageScope();

    const bool fullSizeCopy = IsFullBufferOverwrittenInTextureToBufferCopy(src, dst, copyExtent);
    // Skip clearing the buffer if this is full size copy.
    dst.buffer->SetInitialized(fullSizeCopy || dst.buffer->IsInitialized());

    Ref<BufferBase> destinationBuffer = dst.buffer.Get();
    const uint32_t bytesPerRow = dst.bytesPerRow == wgpu::kCopyStrideUndefined
                                     ? (copyExtent.width * bytesPerTexel)
                                     : dst.bytesPerRow;
    const uint32_t rowsPerImage =
        dst.rowsPerImage == wgpu::kCopyStrideUndefined ? copyExtent.height : dst.rowsPerImage;
    const uint64_t numBytesToCopy =
        ComputeRequiredBytesInCopy(blockInfo, copyExtent, bytesPerRow, rowsPerImage)
            .AcquireSuccess();
    const uint64_t shaderEndOffset = shaderStartOffset + numBytesToCopy;
    const uint64_t shaderBindingSize = Align(shaderEndOffset, 4);
    const bool needsTempForOOBU32Write =
        shaderBindingOffset + shaderBindingSize > dst.buffer->GetSize();
    const bool needsTempForStorageUsage =
        !(dst.buffer->GetInternalUsage() & (kInternalStorageBuffer | wgpu::BufferUsage::Storage));
    const bool useIntermediateCopyBuffer = needsTempForOOBU32Write || needsTempForStorageUsage;
    // Offset to copy to original buffer. Only relevant if useIntermediateCopyBuffer is true.
    uint64_t offsetInOriginalBuf = 0;
    if (useIntermediateCopyBuffer) {
        // If we copy from a texture with (width * texelByteSize) % 4 != 0, to a compact buffer,
        // when we copy the last texel, we inevitably need to access an out of bounds location given
        // by dst.buffer.size as we use array<u32> in the shader for the storage buffer. Although
        // the allocated size of dst.buffer is aligned to 4 bytes by the backend, the size of the
        // storage buffer binding for the shader is not. Thus we make an intermediate buffer aligned
        // to 4 bytes for the compute shader to safely access, and perform an additional buffer to
        // buffer copy at the end. This path should be hit rarely.
        //
        // We also allocate an intermediate buffer if the destination buffer doesn't have Storage
        // usage.
        offsetInOriginalBuf = shaderBindingOffset;
        shaderBindingOffset = 0;

        BufferDescriptor descriptor = {};
        descriptor.size = shaderBindingSize;
        // TODO(dawn:1485): adding CopyDst usage to add kInternalStorageBuffer usage internally.
        descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        DAWN_TRY_ASSIGN(destinationBuffer, device->CreateBuffer(&descriptor));

        const uint64_t paddingSize = shaderBindingSize - shaderEndOffset;
        // Don't clear the temp buffer. Its bytes are either written in shader or copied from
        // the original buffer.
        destinationBuffer->SetInitialized(true);
        if (paddingSize > 0) {
            DAWN_ASSERT(paddingSize < 4);
            // We only need to initialize the last 4 bytes in the temp buffer.
            std::array<uint8_t, 4> clearData = {};
            commandEncoder->APIWriteBuffer(destinationBuffer.Get(),
                                           destinationBuffer->GetSize() - 4, clearData.data(), 4);
        }

        // Copy the bytes that we won't write in the shader (those before offset, padding bytes,
        // etc).
        if (!fullSizeCopy) {
            if (bytesPerRow == copyExtent.width * bytesPerTexel &&
                rowsPerImage == copyExtent.height) {
                // If the copy is compact, we only need to copy from the original buffer:
                // - the first bytes before offset.
                // - the last bytes past the desired copy region.
                if (shaderStartOffset > 0) {
                    commandEncoder->InternalCopyBufferToBufferWithAllocatedSize(
                        dst.buffer.Get(), /*srcOffset=*/offsetInOriginalBuf,
                        destinationBuffer.Get(),
                        /*dstOffset=*/0, /*size=*/Align(shaderStartOffset, 4));
                }
                if (shaderEndOffset != shaderBindingSize) {
                    const auto mod = shaderEndOffset % 4;
                    const auto lastShaderU32Offset = shaderEndOffset - mod;
                    commandEncoder->InternalCopyBufferToBufferWithAllocatedSize(
                        dst.buffer.Get(),
                        /*srcOffset=*/offsetInOriginalBuf + lastShaderU32Offset,
                        destinationBuffer.Get(), /*dstOffset=*/lastShaderU32Offset,
                        /*size=*/shaderBindingSize - lastShaderU32Offset);
                }
            } else {
                commandEncoder->InternalCopyBufferToBufferWithAllocatedSize(
                    dst.buffer.Get(), /*srcOffset=*/offsetInOriginalBuf, destinationBuffer.Get(),
                    /*dstOffset=*/0, shaderBindingSize);
            }
        }
    }

    Ref<BufferBase> uniformBuffer;
    {
        BufferDescriptor bufferDesc = {};
        // Uniform buffer size needs to be multiple of 16 bytes
        bufferDesc.size = sizeof(uint32_t) * 20;
        bufferDesc.usage = wgpu::BufferUsage::Uniform;
        bufferDesc.mappedAtCreation = true;

        {
            IgnoreLazyClearCountScope ignoreClearScope(device);
            DAWN_TRY_ASSIGN(uniformBuffer, device->CreateBuffer(&bufferDesc));
        }

        uint32_t* params =
            static_cast<uint32_t*>(uniformBuffer->GetMappedRange(0, bufferDesc.size));
        // srcOrigin: vec3u
        params[0] = src.origin.x;
        params[1] = src.origin.y;
        params[2] = src.origin.z;

        // packTexelCount: number of texel values (1, 2, or 4) one thread packs into the dst
        // buffer
        params[3] = std::max(1u, 4 / bytesPerTexel);
        // srcExtent: vec3u
        params[4] = copyExtent.width;
        params[5] = copyExtent.height;
        params[6] = copyExtent.depthOrArrayLayers;

        params[7] = src.mipLevel;

        params[8] = bytesPerRow;
        params[9] = rowsPerImage;
        params[10] = shaderStartOffset;

        // These params are only used for formats smaller than 4 bytes
        params[11] = (shaderStartOffset % 4) / bytesPerTexel;  // shift

        params[16] = bytesPerTexel;
        params[17] = numU32PerRowNeedsWriting;
        params[18] = readPreviousRow ? 1 : 0;
        params[19] = rowsPerImage == copyExtent.height ? 1 : 0;  // isCompactImage

        if (textureViewDimension == wgpu::TextureViewDimension::Cube) {
            // cube need texture size to convert texel coord to sample location
            auto levelSize =
                src.texture->GetMipLevelSingleSubresourceVirtualSize(src.mipLevel, Aspect::Color);
            params[12] = levelSize.width;
            params[13] = levelSize.height;
            params[14] = levelSize.depthOrArrayLayers;
        }

        DAWN_TRY(uniformBuffer->Unmap());
    }

    TextureViewDescriptor viewDesc = {};
    switch (src.aspect) {
        case Aspect::Color:
            viewDesc.aspect = wgpu::TextureAspect::All;
            break;
        case Aspect::Depth:
            viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
            break;
        case Aspect::Stencil:
            viewDesc.aspect = wgpu::TextureAspect::StencilOnly;
            break;
        default:
            DAWN_UNREACHABLE();
    }

    viewDesc.dimension = textureViewDimension;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = src.texture->GetNumMipLevels();
    viewDesc.baseArrayLayer = 0;
    if (viewDesc.dimension == wgpu::TextureViewDimension::e2DArray ||
        viewDesc.dimension == wgpu::TextureViewDimension::Cube) {
        viewDesc.arrayLayerCount = src.texture->GetArrayLayers();
    } else {
        viewDesc.arrayLayerCount = 1;
    }

    Ref<TextureViewBase> srcView;
    DAWN_TRY_ASSIGN(srcView, src.texture->CreateView(&viewDesc));

    Ref<BindGroupLayoutBase> bindGroupLayout0;
    DAWN_TRY_ASSIGN(bindGroupLayout0, pipeline->GetBindGroupLayout(0));
    Ref<BindGroupBase> bindGroup0;
    DAWN_TRY_ASSIGN(
        bindGroup0,
        utils::MakeBindGroup(device, bindGroupLayout0,
                             {
                                 {0, srcView},
                                 {1, destinationBuffer, shaderBindingOffset, shaderBindingSize},
                                 {2, uniformBuffer},
                             },
                             UsageValidationMode::Internal));

    Ref<BindGroupLayoutBase> bindGroupLayout1;
    Ref<BindGroupBase> bindGroup1;
    if (textureViewDimension == wgpu::TextureViewDimension::Cube) {
        // Cube texture requires an extra sampler to call textureSampleLevel
        DAWN_TRY_ASSIGN(bindGroupLayout1, pipeline->GetBindGroupLayout(1));

        SamplerDescriptor samplerDesc = {};
        Ref<SamplerBase> sampler;
        DAWN_TRY_ASSIGN(sampler, device->CreateSampler(&samplerDesc));

        DAWN_TRY_ASSIGN(bindGroup1, utils::MakeBindGroup(device, bindGroupLayout1,
                                                         {
                                                             {0, sampler},
                                                         },
                                                         UsageValidationMode::Internal));
    }

    Ref<ComputePassEncoder> pass = commandEncoder->BeginComputePass();
    pass->APISetPipeline(pipeline.Get());
    pass->APISetBindGroup(0, bindGroup0.Get());
    if (textureViewDimension == wgpu::TextureViewDimension::Cube) {
        pass->APISetBindGroup(1, bindGroup1.Get());
    }
    pass->APIDispatchWorkgroups(workgroupCountX, workgroupCountY, workgroupCountZ);
    pass->APIEnd();

    if (useIntermediateCopyBuffer) {
        DAWN_ASSERT(destinationBuffer->GetSize() <= dst.buffer->GetAllocatedSize());
        commandEncoder->InternalCopyBufferToBufferWithAllocatedSize(
            destinationBuffer.Get(), /*srcOffset=*/0, dst.buffer.Get(),
            /*dstOffset=*/offsetInOriginalBuf, shaderBindingSize);
    }

    return {};
}

}  // namespace dawn::native
