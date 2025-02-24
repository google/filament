//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BlitResolve.frag: Blit color or depth/stencil images, or resolve multisampled ones.

#version 450 core

#extension GL_GOOGLE_include_directive : require

#define MAKE_SRC_RESOURCE(prefix, type) prefix ## type

#if BlitColorFloat

#define IsBlitColor 1
#define COLOR_SRC_RESOURCE(type) type
#define ColorType vec4

#elif BlitColorInt

#define IsBlitColor 1
#define COLOR_SRC_RESOURCE(type) MAKE_SRC_RESOURCE(i, type)
#define ColorType ivec4

#elif BlitColorUint

#define IsBlitColor 1
#define COLOR_SRC_RESOURCE(type) MAKE_SRC_RESOURCE(u, type)
#define ColorType uvec4

#elif BlitDepth

#define IsBlitDepth 1

#elif BlitStencil

#define IsBlitStencil 1

#elif BlitDepthStencil

#define IsBlitDepth 1
#define IsBlitStencil 1

#else

#error "Not all resolve targets are accounted for"

#endif

#if IsBlitColor && (IsBlitDepth || IsBlitStencil)
#error "The shader doesn't blit color and depth/stencil at the same time."
#endif

#if IsResolve
#extension GL_EXT_samplerless_texture_functions : require
#endif
#if IsBlitStencil
#extension GL_ARB_shader_stencil_export : require
#endif

#define DEPTH_SRC_RESOURCE(type) type
#define STENCIL_SRC_RESOURCE(type) MAKE_SRC_RESOURCE(u, type)

#if IsResolve

#define CoordType ivec2
#if SrcIsArray
#define SRC_RESOURCE_NAME texture2DMSArray
#define TEXEL_FETCH(src, coord, sample) texelFetch(src, ivec3(coord, params.srcLayer), sample)
#else
#define SRC_RESOURCE_NAME texture2DMS
#define TEXEL_FETCH(src, coord, sample) texelFetch(src, coord, sample)
#endif

#define COLOR_TEXEL_FETCH(src, coord, sample) TEXEL_FETCH(src, coord, sample)
#define DEPTH_TEXEL_FETCH(src, coord, sample) TEXEL_FETCH(src, coord, sample)
#define STENCIL_TEXEL_FETCH(src, coord, sample) TEXEL_FETCH(src, coord, sample)

#else

#define CoordType vec2
#if SrcIsArray
#define SRC_RESOURCE_NAME texture2DArray
#define SRC_SAMPLER_NAME sampler2DArray
#define TEXEL_FETCH(src, coord, sample) texture(src, vec3(coord * params.invSrcExtent, params.srcLayer))
#else
#define SRC_RESOURCE_NAME texture2D
#define SRC_SAMPLER_NAME sampler2D
#define TEXEL_FETCH(src, coord, sample) texture(src, coord * params.invSrcExtent)
#endif

#define COLOR_TEXEL_FETCH(src, coord, sample) TEXEL_FETCH(COLOR_SRC_RESOURCE(SRC_SAMPLER_NAME)(src, blitSampler), coord, sample)
#define DEPTH_TEXEL_FETCH(src, coord, sample) TEXEL_FETCH(DEPTH_SRC_RESOURCE(SRC_SAMPLER_NAME)(src, blitSampler), coord, sample)
#define STENCIL_TEXEL_FETCH(src, coord, sample) TEXEL_FETCH(STENCIL_SRC_RESOURCE(SRC_SAMPLER_NAME)(src, blitSampler), coord, sample)

#endif  // IsResolve

#if IsBlitColor
layout(set = 0, binding = 0) uniform COLOR_SRC_RESOURCE(SRC_RESOURCE_NAME) color;

layout(location = 0) out ColorType colorOut0;
layout(location = 1) out ColorType colorOut1;
layout(location = 2) out ColorType colorOut2;
layout(location = 3) out ColorType colorOut3;
layout(location = 4) out ColorType colorOut4;
layout(location = 5) out ColorType colorOut5;
layout(location = 6) out ColorType colorOut6;
layout(location = 7) out ColorType colorOut7;
#endif
#if IsBlitDepth
layout(set = 0, binding = 0) uniform DEPTH_SRC_RESOURCE(SRC_RESOURCE_NAME) depth;
#endif
#if IsBlitStencil
layout(set = 0, binding = 1) uniform STENCIL_SRC_RESOURCE(SRC_RESOURCE_NAME) stencil;
#endif

#if !IsResolve
layout(set = 0, binding = 2) uniform sampler blitSampler;
#endif

#include "BlitResolve.inc"

void main()
{
    CoordType srcImageCoords = getSrcImageCoords();

#if IsBlitColor
#if IsResolve
    ColorType colorValue = ColorType(0, 0, 0, 0);
    for (int i = 0; i < params.samples; ++i)
    {
        colorValue += COLOR_TEXEL_FETCH(color, srcImageCoords, i);
    }
#if BlitColorFloat
    colorValue *= params.invSamples;
#else
    colorValue = ColorType(round(colorValue * params.invSamples));
#endif

#else
    ColorType colorValue = COLOR_TEXEL_FETCH(color, srcImageCoords, 0);
#endif

    broadcastColor(colorValue);
#endif  // IsBlitColor

    // Note: always resolve depth/stencil using sample 0.  GLES3 gives us freedom in choosing how
    // to resolve depth/stencil images.

#if IsBlitDepth
    gl_FragDepth = DEPTH_TEXEL_FETCH(depth, srcImageCoords, 0).x;
#endif  // IsBlitDepth

#if IsBlitStencil
    gl_FragStencilRefARB = int(STENCIL_TEXEL_FETCH(stencil, srcImageCoords, 0).x);
#endif  // IsBlitStencil
}
