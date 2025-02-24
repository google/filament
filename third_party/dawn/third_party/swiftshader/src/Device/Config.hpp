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

#ifndef sw_Config_hpp
#define sw_Config_hpp

namespace sw {

constexpr int OUTLINE_RESOLUTION = 8192;  // Maximum vertical resolution of the render target
constexpr int MIPMAP_LEVELS = 15;
constexpr int MAX_CLIP_DISTANCES = 8;
constexpr int MAX_CULL_DISTANCES = 8;
constexpr int MIN_TEXEL_OFFSET = -8;
constexpr int MAX_TEXEL_OFFSET = 7;
constexpr int MAX_TEXTURE_LOD = MIPMAP_LEVELS - 2;  // Trilinear accesses lod+1
constexpr int MAX_COLOR_BUFFERS = 8;
constexpr int MAX_INTERFACE_COMPONENTS = 32 * 4;  // Must be multiple of 4 for 16-byte alignment.
constexpr int MAX_FRAMEBUFFER_DIM = OUTLINE_RESOLUTION;
constexpr int MAX_VIEWPORT_DIM = MAX_FRAMEBUFFER_DIM;

}  // namespace sw

#endif  // sw_Config_hpp
