//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Blit3DSrc.frag: Blit 3D color images.

#version 450 core

#extension GL_GOOGLE_include_directive : require

#define MAKE_SRC_RESOURCE(prefix, type) prefix ## type

#define IsBlitColor 1

#if BlitFloat

#define COLOR_SRC_RESOURCE(type) type
#define ColorType vec4

#elif BlitInt

#define COLOR_SRC_RESOURCE(type) MAKE_SRC_RESOURCE(i, type)
#define ColorType ivec4

#elif BlitUint

#define COLOR_SRC_RESOURCE(type) MAKE_SRC_RESOURCE(u, type)
#define ColorType uvec4

#else

#error "Not all resolve targets are accounted for"

#endif

#define CoordType vec2
#define SRC_RESOURCE_NAME texture3D
#define SRC_SAMPLER_NAME sampler3D
#define TEXEL_FETCH(src, coord, sample) texture(src, vec3((coord.xy) * params.invSrcExtent, coord.z))

#define COLOR_TEXEL_FETCH(src, coord, sample) TEXEL_FETCH(COLOR_SRC_RESOURCE(SRC_SAMPLER_NAME)(src, blitSampler), coord, sample)

layout(set = 0, binding = 0) uniform COLOR_SRC_RESOURCE(SRC_RESOURCE_NAME) color;

layout(location = 0) out ColorType colorOut0;
layout(location = 1) out ColorType colorOut1;
layout(location = 2) out ColorType colorOut2;
layout(location = 3) out ColorType colorOut3;
layout(location = 4) out ColorType colorOut4;
layout(location = 5) out ColorType colorOut5;
layout(location = 6) out ColorType colorOut6;
layout(location = 7) out ColorType colorOut7;

layout(set = 0, binding = 2) uniform sampler blitSampler;

#include "BlitResolve.inc"

void main()
{
    CoordType srcImageCoordsXY = getSrcImageCoords();
    vec3 srcImageCoords = vec3(srcImageCoordsXY, params.srcLayer);

    ColorType colorValue = COLOR_TEXEL_FETCH(color, srcImageCoords, 0);

    broadcastColor(colorValue);
}
