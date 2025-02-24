//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferFetchTest:
//   Tests the correctness of the EXT_shader_framebuffer_fetch and the
//   EXT_shader_framebuffer_fetch_non_coherent extensions.
//

#include "common/debug.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

namespace angle
{
//
// Shared Vertex Shaders for the tests below
//
// A 1.0 GLSL vertex shader
static constexpr char k100VS[] = R"(#version 100
attribute vec4 a_position;

void main (void)
{
    gl_Position = a_position;
})";

// A 3.1 GLSL vertex shader
static constexpr char k310VS[] = R"(#version 310 es
in highp vec4 a_position;

void main (void)
{
    gl_Position = a_position;
})";

// Shared simple (i.e. no framebuffer fetch) Fragment Shaders for the tests below
//
// Simple (i.e. no framebuffer fetch) 3.1 GLSL fragment shader that writes to 1 attachment
static constexpr char k310NoFetch1AttachmentFS[] = R"(#version 310 es
layout(location = 0) out highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color;
})";

// Shared Coherent Fragment Shaders for the tests below
//
// Coherent version of a 1.0 GLSL fragment shader that uses gl_LastFragData
static constexpr char k100CoherentFS[] = R"(#version 100
#extension GL_EXT_shader_framebuffer_fetch : require
mediump vec4 gl_LastFragData[gl_MaxDrawBuffers];
uniform highp vec4 u_color;

void main (void)
{
    gl_FragColor = u_color + gl_LastFragData[0];
})";

// Coherent version of a 3.1 GLSL fragment shader that writes to 1 attachment
static constexpr char k310Coherent1AttachmentFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) inout highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color += u_color;
})";

// Coherent version of a 3.1 GLSL fragment shader that writes the output to a storage buffer.
static constexpr char k310CoherentStorageBuffer[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) inout highp vec4 o_color;

layout(std140, binding = 0) buffer outBlock {
    highp vec4 data[256];
};

uniform highp vec4 u_color;
void main (void)
{
    uint index = uint(gl_FragCoord.y) * 16u + uint(gl_FragCoord.x);
    data[index] = o_color;
    o_color += u_color;
})";

// Coherent version of a 1.0 GLSL fragment shader that writes to 4 attachments with constant indices
static constexpr char k100Coherent4AttachmentFS[] = R"(#version 100
#extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_EXT_draw_buffers : require
uniform highp vec4 u_color;

void main (void)
{
    gl_FragData[0] = gl_LastFragData[0] + u_color;
    gl_FragData[1] = gl_LastFragData[1] + u_color;
    gl_FragData[2] = gl_LastFragData[2] + u_color;
    gl_FragData[3] = gl_LastFragData[3] + u_color;
})";

// Coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments
static constexpr char k310Coherent4AttachmentFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) inout highp vec4 o_color0;
layout(location = 1) inout highp vec4 o_color1;
layout(location = 2) inout highp vec4 o_color2;
layout(location = 3) inout highp vec4 o_color3;
uniform highp vec4 u_color;

void main (void)
{
    o_color0 += u_color;
    o_color1 += u_color;
    o_color2 += u_color;
    o_color3 += u_color;
})";

// Coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments via an inout
// array
static constexpr char k310Coherent4AttachmentArrayFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
inout highp vec4 o_color[4];
uniform highp vec4 u_color;

void main (void)
{
    o_color[0] += u_color;
    o_color[1] += u_color;
    o_color[2] += u_color;
    o_color[3] += u_color;
})";

// Coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments with the order of
// non-fetch program and fetch program with different attachments (version 1)
static constexpr char k310CoherentDifferent4AttachmentFS1[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) inout highp vec4 o_color0;
layout(location = 1) out highp vec4 o_color1;
layout(location = 2) inout highp vec4 o_color2;
layout(location = 3) out highp vec4 o_color3;
uniform highp vec4 u_color;

void main (void)
{
    o_color0 += u_color;
    o_color1 = u_color;
    o_color2 += u_color;
    o_color3 = u_color;
})";

// Coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments with the order
// of non-fetch program and fetch program with different attachments (version 2)
static constexpr char k310CoherentDifferent4AttachmentFS2[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) inout highp vec4 o_color0;
layout(location = 1) out highp vec4 o_color1;
layout(location = 2) out highp vec4 o_color2;
layout(location = 3) inout highp vec4 o_color3;
uniform highp vec4 u_color;

void main (void)
{
    o_color0 += u_color;
    o_color1 = u_color;
    o_color2 = u_color;
    o_color3 += u_color;
})";

// Coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments, fetching from
// different indices (version 3)
static constexpr char k310CoherentDifferent4AttachmentFS3[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) out highp vec4 o_color0;
layout(location = 1) inout highp vec4 o_color1;
layout(location = 2) inout highp vec4 o_color2;
layout(location = 3) out highp vec4 o_color3;
uniform highp vec4 u_color;

void main (void)
{
    o_color0 = u_color;
    o_color1 += u_color;
    o_color2 += u_color;
    o_color3 = u_color;
})";

// Coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments, fetching from
// different indices (version 4)
static constexpr char k310CoherentDifferent4AttachmentFS4[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) out highp vec4 o_color0;
layout(location = 1) out highp vec4 o_color1;
layout(location = 2) inout highp vec4 o_color2;
layout(location = 3) inout highp vec4 o_color3;
uniform highp vec4 u_color;

void main (void)
{
    o_color0 = u_color;
    o_color1 = u_color;
    o_color2 += u_color;
    o_color3 += u_color;
})";

// Coherent version of a 1.0 GLSL fragment shader with complex interactions
static constexpr char k100CoherentComplexFS[] = R"(#version 100
#extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_EXT_draw_buffers : require
precision highp float;
uniform vec4 u_color;

vec4 addColor(vec4 lastFragData, vec4 color)
{
    return lastFragData + color;
}

void addLastFragData(inout vec4 outVar, vec4 lastFragData)
{
    outVar += lastFragData;
}

void main (void)
{
    // Leave gl_LastFragData[0] unused, as well as gl_LastFragData[2]
    gl_FragData[0] = u_color;
    gl_FragData[1] = addColor(gl_LastFragData[1], u_color);
    gl_FragData[2] = u_color;
    gl_FragData[3] = addColor(gl_LastFragData[3], u_color);

    // Make sure gl_LastFragData is not clobbered by a write to gl_FragData.
    gl_FragData[1] -= gl_LastFragData[1];
    gl_FragData[3] -= gl_LastFragData[3];
    // Test passing to inout variables.
    addLastFragData(gl_FragData[1], gl_LastFragData[1]);
    addLastFragData(gl_FragData[3], gl_LastFragData[3]);
})";

// Coherent version of a 3.1 GLSL fragment shader with complex interactions
static constexpr char k310CoherentComplexFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
precision highp float;
layout(location = 0) inout highp vec4 o_color0;
layout(location = 1) inout highp vec4 o_color1;
layout(location = 2) inout highp vec4 o_color2[2];
uniform vec4 u_color;

vec4 addColor(vec4 lastValue, vec4 color)
{
    return lastValue + color;
}

vec4 getColor2_1()
{
    return o_color2[1];
}

void addUniform(inout vec4 outVar)
{
    outVar += u_color;
}

void main (void)
{
    // o_color0 and o_color2[0] don't use the input value.
    o_color0 = u_color;
    o_color2[0] = u_color;

    addUniform(o_color1);
    addUniform(o_color2[1]);

    // Make sure reading back from the output variables returns the latest value and not the
    // original input value.
    vec4 temp1 = o_color1;
    vec4 temp3 = getColor2_1();

    o_color1 = temp1;
    o_color2[1] = temp3;
})";

// Shared Non-Coherent Fragment Shaders for the tests below
//
// Non-coherent version of a 1.0 GLSL fragment shader that uses gl_LastFragData
static constexpr char k100NonCoherentFS[] = R"(#version 100
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(noncoherent) mediump vec4 gl_LastFragData[gl_MaxDrawBuffers];
uniform highp vec4 u_color;

void main (void)
{
    gl_FragColor = u_color + gl_LastFragData[0];
})";

// Non-coherent version of a 3.1 GLSL fragment shader that writes to 1 attachment
static constexpr char k310NonCoherent1AttachmentFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(noncoherent, location = 0) inout highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color += u_color;
})";

// Non-coherent version of a 3.1 GLSL fragment shader that writes the output to a storage buffer.
static constexpr char k310NonCoherentStorageBuffer[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(noncoherent) inout highp vec4 o_color;

layout(std140, binding = 0) buffer outBlock {
    highp vec4 data[256];
};

uniform highp vec4 u_color;
void main (void)
{
    uint index = uint(gl_FragCoord.y) * 16u + uint(gl_FragCoord.x);
    data[index] = o_color;
    o_color += u_color;
})";

// Non-coherent version of a 1.0 GLSL fragment shader that writes to 4 attachments with constant
// indices
static constexpr char k100NonCoherent4AttachmentFS[] = R"(#version 100
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
#extension GL_EXT_draw_buffers : require
layout(noncoherent) mediump vec4 gl_LastFragData[gl_MaxDrawBuffers];
uniform highp vec4 u_color;

void main (void)
{
    gl_FragData[0] = gl_LastFragData[0] + u_color;
    gl_FragData[1] = gl_LastFragData[1] + u_color;
    gl_FragData[2] = gl_LastFragData[2] + u_color;
    gl_FragData[3] = gl_LastFragData[3] + u_color;
})";

// Non-coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments
static constexpr char k310NonCoherent4AttachmentFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(noncoherent, location = 0) inout highp vec4 o_color0;
layout(noncoherent, location = 1) inout highp vec4 o_color1;
layout(noncoherent, location = 2) inout highp vec4 o_color2;
layout(noncoherent, location = 3) inout highp vec4 o_color3;
uniform highp vec4 u_color;

void main (void)
{
    o_color0 += u_color;
    o_color1 += u_color;
    o_color2 += u_color;
    o_color3 += u_color;
})";

// Non-coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments via an inout
// array
static constexpr char k310NonCoherent4AttachmentArrayFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(noncoherent, location = 0) inout highp vec4 o_color[4];
uniform highp vec4 u_color;

void main (void)
{
    o_color[0] += u_color;
    o_color[1] += u_color;
    o_color[2] += u_color;
    o_color[3] += u_color;
})";

// Non-coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments with the order
// of non-fetch program and fetch program with different attachments (version 1)
static constexpr char k310NonCoherentDifferent4AttachmentFS1[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(noncoherent, location = 0) inout highp vec4 o_color0;
layout(location = 1) out highp vec4 o_color1;
layout(noncoherent, location = 2) inout highp vec4 o_color2;
layout(location = 3) out highp vec4 o_color3;
uniform highp vec4 u_color;

void main (void)
{
    o_color0 += u_color;
    o_color1 = u_color;
    o_color2 += u_color;
    o_color3 = u_color;
})";

// Non-coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments with the order
// of non-fetch program and fetch program with different attachments (version 2)
static constexpr char k310NonCoherentDifferent4AttachmentFS2[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(noncoherent, location = 0) inout highp vec4 o_color0;
layout(location = 1) out highp vec4 o_color1;
layout(location = 2) out highp vec4 o_color2;
layout(noncoherent, location = 3) inout highp vec4 o_color3;
uniform highp vec4 u_color;

void main (void)
{
    o_color0 += u_color;
    o_color1 = u_color;
    o_color2 = u_color;
    o_color3 += u_color;
})";

// Non-coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments, fetching from
// different indices (version 3)
static constexpr char k310NonCoherentDifferent4AttachmentFS3[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(location = 0) out highp vec4 o_color0;
layout(noncoherent, location = 1) inout highp vec4 o_color1;
layout(noncoherent, location = 2) inout highp vec4 o_color2;
layout(location = 3) out highp vec4 o_color3;
uniform highp vec4 u_color;

void main (void)
{
    o_color0 = u_color;
    o_color1 += u_color;
    o_color2 += u_color;
    o_color3 = u_color;
})";

// Non-coherent version of a 3.1 GLSL fragment shader that writes to 4 attachments, fetching from
// different indices (version 4)
static constexpr char k310NonCoherentDifferent4AttachmentFS4[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(location = 0) out highp vec4 o_color0;
layout(location = 1) out highp vec4 o_color1;
layout(noncoherent, location = 2) inout highp vec4 o_color2;
layout(noncoherent, location = 3) inout highp vec4 o_color3;
uniform highp vec4 u_color;

void main (void)
{
    o_color0 = u_color;
    o_color1 = u_color;
    o_color2 += u_color;
    o_color3 += u_color;
})";

// Non-coherent version of a 1.0 GLSL fragment shader with complex interactions
static constexpr char k100NonCoherentComplexFS[] = R"(#version 100
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
#extension GL_EXT_draw_buffers : require
precision highp float;
layout(noncoherent) mediump vec4 gl_LastFragData[gl_MaxDrawBuffers];
uniform vec4 u_color;

vec4 addColor(vec4 lastFragData, vec4 color)
{
    return lastFragData + color;
}

void addLastFragData(inout vec4 outVar, vec4 lastFragData)
{
    outVar += lastFragData;
}

void main (void)
{
    // Leave gl_LastFragData[0] unused, as well as gl_LastFragData[2]
    gl_FragData[0] = u_color;
    gl_FragData[1] = addColor(gl_LastFragData[1], u_color);
    gl_FragData[2] = u_color;
    gl_FragData[3] = addColor(gl_LastFragData[3], u_color);

    // Make sure gl_LastFragData is not clobbered by a write to gl_FragData.
    gl_FragData[1] -= gl_LastFragData[1];
    gl_FragData[3] -= gl_LastFragData[3];
    // Test passing to inout variables.
    addLastFragData(gl_FragData[1], gl_LastFragData[1]);
    addLastFragData(gl_FragData[3], gl_LastFragData[3]);
})";

// Non-coherent version of a 3.1 GLSL fragment shader with complex interactions
static constexpr char k310NonCoherentComplexFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
precision highp float;
layout(location = 0) out highp vec4 o_color0;
layout(noncoherent, location = 1) inout highp vec4 o_color1;
layout(noncoherent, location = 2) inout highp vec4 o_color2[2];
uniform vec4 u_color;

vec4 addColor(vec4 lastValue, vec4 color)
{
    return lastValue + color;
}

vec4 getColor2_1()
{
    return o_color2[1];
}

void addUniform(inout vec4 outVar)
{
    outVar += u_color;
}

void main (void)
{
    // o_color0 and o_color2[0] don't use the input value.
    o_color0 = u_color;
    o_color2[0] = u_color;

    addUniform(o_color1);
    addUniform(o_color2[1]);

    // Make sure reading back from the output variables returns the latest value and not the
    // original input value.
    vec4 temp1 = o_color1;
    vec4 temp3 = getColor2_1();

    o_color1 = temp1;
    o_color2[1] = temp3;
})";

// Shared Coherent Fragment Shaders for the tests below
//
// Coherent version of a 1.0 GLSL fragment shader that uses gl_LastFragColorARM
static constexpr char k100ARMFS[] = R"(#version 100
#extension GL_ARM_shader_framebuffer_fetch : require
mediump vec4 gl_LastFragColorARM;
uniform highp vec4 u_color;

void main (void)
{
    gl_FragColor = u_color + gl_LastFragColorARM;
})";

// ARM version of a 3.1 GLSL fragment shader that writes to 1 attachment
static constexpr char k310ARM1AttachmentFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch : require
layout(location = 0) out highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color + gl_LastFragColorARM;
})";

// ARM version of a 3.1 GLSL fragment shader that writes the output to a storage buffer.
static constexpr char k310ARMStorageBuffer[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch : require
layout(location = 0) out highp vec4 o_color;

layout(std140, binding = 0) buffer outBlock {
    highp vec4 data[256];
};

uniform highp vec4 u_color;
void main (void)
{
    uint index = uint(gl_FragCoord.y) * 16u + uint(gl_FragCoord.x);
    data[index] = gl_LastFragColorARM;
    o_color = u_color + gl_LastFragColorARM;
})";

// Variants that use both EXT and ARM simultaneously.  At least one app has been observed to do
// this.
static constexpr char k100BothFS[] = R"(#version 100
#extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_ARM_shader_framebuffer_fetch : require
uniform highp vec4 u_color;

void main (void)
{
    gl_FragColor = u_color + (gl_LastFragColorARM + gl_LastFragData[0]) / 2.;
})";

static constexpr char k310Both1AttachmentFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_ARM_shader_framebuffer_fetch : require
inout highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color + (o_color + gl_LastFragColorARM) / 2.;
})";

static constexpr char k100Both4AttachmentFS[] = R"(#version 100
#extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_ARM_shader_framebuffer_fetch : require
#extension GL_EXT_draw_buffers : require
uniform highp vec4 u_color;

void main (void)
{
    gl_FragData[0] = (gl_LastFragData[0] + gl_LastFragColorARM) / 2. + u_color;
    gl_FragData[1] = gl_LastFragData[1] + u_color;
    gl_FragData[2] = gl_LastFragData[2] + u_color;
    gl_FragData[3] = gl_LastFragData[3] + u_color;
})";

static constexpr char k100BothComplexFS[] = R"(#version 100
#extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_ARM_shader_framebuffer_fetch : require
#extension GL_EXT_draw_buffers : require
precision highp float;
uniform vec4 u_color;

vec4 addColor(vec4 lastFragData, vec4 color)
{
    return lastFragData + color;
}

void addLastFragData(inout vec4 outVar, vec4 lastFragData)
{
    outVar += lastFragData;
}

void main (void)
{
    // Leave gl_LastFragData[1] unused, as well as gl_LastFragData[3]
    gl_FragData[0] = addColor((gl_LastFragData[0] + gl_LastFragColorARM) / 2., u_color);
    gl_FragData[1] = u_color;
    gl_FragData[2] = addColor(gl_LastFragData[2], u_color);
    gl_FragData[3] = u_color;

    // Make sure gl_LastFragData is not clobbered by a write to gl_FragData.
    gl_FragData[0] -= gl_LastFragColorARM;
    gl_FragData[2] -= gl_LastFragData[2];
    // Test passing to inout variables.
    addLastFragData(gl_FragData[0], gl_LastFragData[0]);
    addLastFragData(gl_FragData[2], gl_LastFragData[2]);
})";

static constexpr char k310BothComplexFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_ARM_shader_framebuffer_fetch : require
precision highp float;
layout(location = 0) inout highp vec4 o_color0;
layout(location = 1) inout highp vec4 o_color1;
layout(location = 2) inout highp vec4 o_color2[2];
uniform vec4 u_color;

vec4 addColor(vec4 lastValue, vec4 color)
{
    return lastValue + color;
}

vec4 getColor2_0()
{
    return o_color2[0];
}

void addUniform(inout vec4 outVar)
{
    outVar += u_color;
}

void main (void)
{
    // o_color1 and o_color2[1] don't use the input value.
    o_color1 = u_color;
    o_color2[1] = u_color;

    o_color0 = gl_LastFragColorARM + u_color;
    addUniform(o_color2[0]);

    // Make sure reading back from the output variables returns the latest value and not the
    // original input value.
    vec4 temp0 = o_color0;
    vec4 temp2 = getColor2_0();

    o_color0 = temp0;
    o_color2[0] = temp2;

    // Make sure gl_LastFragColorARM is not clobberred by the write to o_color0
    if (gl_LastFragColorARM == o_color0)
        o_color0 = vec4(0);
})";

class FramebufferFetchES31 : public ANGLETest<>
{
  protected:
    static constexpr GLuint kMaxColorBuffer = 4u;
    static constexpr GLuint kViewportWidth  = 16u;
    static constexpr GLuint kViewportHeight = 16u;
    static constexpr GLenum kDSFormat[6]    = {GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24,
                                               GL_DEPTH24_STENCIL8,  GL_DEPTH_COMPONENT32F,
                                               GL_DEPTH32F_STENCIL8, GL_STENCIL_INDEX8};
    FramebufferFetchES31()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);

        mCoherentExtension = false;
        mARMExtension      = false;
        mBothExtensions    = false;
    }

    enum WhichExtension
    {
        COHERENT,
        NON_COHERENT,
        ARM,
        BOTH,
    };
    void setWhichExtension(WhichExtension whichExtension)
    {
        mCoherentExtension = whichExtension != NON_COHERENT;
        mARMExtension      = whichExtension == ARM;
        mBothExtensions    = whichExtension == BOTH;
    }

    enum WhichFragmentShader
    {
        GLSL100,
        GLSL310_NO_FETCH_1ATTACHMENT,
        GLSL310_1ATTACHMENT,
        GLSL310_1ATTACHMENT_WITH_STORAGE_BUFFER,
        GLSL100_4ATTACHMENT,
        GLSL100_COMPLEX,
        GLSL310_4ATTACHMENT,
        GLSL310_4ATTACHMENT_ARRAY,
        GLSL310_4ATTACHMENT_DIFFERENT1,
        GLSL310_4ATTACHMENT_DIFFERENT2,
        GLSL310_4ATTACHMENT_DIFFERENT3,
        GLSL310_4ATTACHMENT_DIFFERENT4,
        GLSL310_COMPLEX,
    };
    const char *getFragmentShader(WhichFragmentShader whichFragmentShader)
    {
        if (mBothExtensions)
        {
            switch (whichFragmentShader)
            {
                case GLSL100:
                    return k100BothFS;
                case GLSL310_NO_FETCH_1ATTACHMENT:
                    return k310NoFetch1AttachmentFS;
                case GLSL310_1ATTACHMENT:
                    return k310Both1AttachmentFS;
                case GLSL100_4ATTACHMENT:
                    return k100Both4AttachmentFS;
                case GLSL100_COMPLEX:
                    return k100BothComplexFS;
                case GLSL310_COMPLEX:
                    return k310BothComplexFS;
                default:
                    UNREACHABLE();
                    return nullptr;
            }
        }
        else if (mARMExtension)
        {
            // gl_LastFragColorARM cannot support multiple attachments
            switch (whichFragmentShader)
            {
                case GLSL100:
                    return k100ARMFS;
                case GLSL310_NO_FETCH_1ATTACHMENT:
                    return k310NoFetch1AttachmentFS;
                case GLSL310_1ATTACHMENT:
                    return k310ARM1AttachmentFS;
                case GLSL310_1ATTACHMENT_WITH_STORAGE_BUFFER:
                    return k310ARMStorageBuffer;
                default:
                    UNREACHABLE();
                    return nullptr;
            }
        }
        else if (mCoherentExtension)
        {
            switch (whichFragmentShader)
            {
                case GLSL100:
                    return k100CoherentFS;
                case GLSL310_NO_FETCH_1ATTACHMENT:
                    return k310NoFetch1AttachmentFS;
                case GLSL310_1ATTACHMENT:
                    return k310Coherent1AttachmentFS;
                case GLSL310_1ATTACHMENT_WITH_STORAGE_BUFFER:
                    return k310CoherentStorageBuffer;
                case GLSL100_4ATTACHMENT:
                    return k100Coherent4AttachmentFS;
                case GLSL310_4ATTACHMENT:
                    return k310Coherent4AttachmentFS;
                case GLSL310_4ATTACHMENT_ARRAY:
                    return k310Coherent4AttachmentArrayFS;
                case GLSL310_4ATTACHMENT_DIFFERENT1:
                    return k310CoherentDifferent4AttachmentFS1;
                case GLSL310_4ATTACHMENT_DIFFERENT2:
                    return k310CoherentDifferent4AttachmentFS2;
                case GLSL310_4ATTACHMENT_DIFFERENT3:
                    return k310CoherentDifferent4AttachmentFS3;
                case GLSL310_4ATTACHMENT_DIFFERENT4:
                    return k310CoherentDifferent4AttachmentFS4;
                case GLSL100_COMPLEX:
                    return k100CoherentComplexFS;
                case GLSL310_COMPLEX:
                    return k310CoherentComplexFS;
                default:
                    UNREACHABLE();
                    return nullptr;
            }
        }
        else
        {
            switch (whichFragmentShader)
            {
                case GLSL100:
                    return k100NonCoherentFS;
                case GLSL310_NO_FETCH_1ATTACHMENT:
                    return k310NoFetch1AttachmentFS;
                case GLSL310_1ATTACHMENT:
                    return k310NonCoherent1AttachmentFS;
                case GLSL310_1ATTACHMENT_WITH_STORAGE_BUFFER:
                    return k310NonCoherentStorageBuffer;
                case GLSL100_4ATTACHMENT:
                    return k100NonCoherent4AttachmentFS;
                case GLSL310_4ATTACHMENT:
                    return k310NonCoherent4AttachmentFS;
                case GLSL310_4ATTACHMENT_ARRAY:
                    return k310NonCoherent4AttachmentArrayFS;
                case GLSL310_4ATTACHMENT_DIFFERENT1:
                    return k310NonCoherentDifferent4AttachmentFS1;
                case GLSL310_4ATTACHMENT_DIFFERENT2:
                    return k310NonCoherentDifferent4AttachmentFS2;
                case GLSL310_4ATTACHMENT_DIFFERENT3:
                    return k310NonCoherentDifferent4AttachmentFS3;
                case GLSL310_4ATTACHMENT_DIFFERENT4:
                    return k310NonCoherentDifferent4AttachmentFS4;
                case GLSL100_COMPLEX:
                    return k100NonCoherentComplexFS;
                case GLSL310_COMPLEX:
                    return k310NonCoherentComplexFS;
                default:
                    UNREACHABLE();
                    return nullptr;
            }
        }
    }

    void render(GLuint coordLoc, GLboolean needsFramebufferFetchBarrier)
    {
        const GLfloat coords[] = {
            -1.0f, -1.0f, +1.0f, -1.0f, +1.0f, +1.0f, -1.0f, +1.0f,
        };

        const GLushort indices[] = {
            0, 1, 2, 2, 3, 0,
        };

        glViewport(0, 0, kViewportWidth, kViewportHeight);

        GLBuffer coordinatesBuffer;
        GLBuffer elementsBuffer;

        glBindBuffer(GL_ARRAY_BUFFER, coordinatesBuffer);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(coords), coords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(coordLoc);
        glVertexAttribPointer(coordLoc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)sizeof(indices), &indices[0],
                     GL_STATIC_DRAW);

        if (needsFramebufferFetchBarrier)
        {
            glFramebufferFetchBarrierEXT();
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

        ASSERT_GL_NO_ERROR();
    }

    void BasicTest(GLProgram &program)
    {
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        std::vector<GLColor> greenColor(kViewportWidth * kViewportHeight, GLColor::green);
        GLTexture colorBufferTex;
        glBindTexture(GL_TEXTURE_2D, colorBufferTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, greenColor.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex,
                               0);

        ASSERT_GL_NO_ERROR();

        float color[4]      = {1.0f, 0.0f, 0.0f, 1.0f};
        GLint colorLocation = glGetUniformLocation(program, "u_color");
        glUniform4fv(colorLocation, 1, color);

        GLint positionLocation = glGetAttribLocation(program, "a_position");
        render(positionLocation, !mCoherentExtension);

        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void MultipleRenderTargetTest(GLProgram &program, WhichFragmentShader whichFragmentShader)
    {
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        std::vector<GLColor> color0(kViewportWidth * kViewportHeight, GLColor::cyan);
        std::vector<GLColor> color1(kViewportWidth * kViewportHeight, GLColor::green);
        std::vector<GLColor> color2(kViewportWidth * kViewportHeight, GLColor::blue);
        std::vector<GLColor> color3(kViewportWidth * kViewportHeight, GLColor::black);
        GLTexture colorBufferTex[kMaxColorBuffer];
        GLenum colorAttachments[kMaxColorBuffer] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                                    GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glBindTexture(GL_TEXTURE_2D, colorBufferTex[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color0.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color1.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color2.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex[3]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color3.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        for (unsigned int i = 0; i < kMaxColorBuffer; i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachments[i], GL_TEXTURE_2D,
                                   colorBufferTex[i], 0);
        }
        glDrawBuffers(kMaxColorBuffer, &colorAttachments[0]);

        ASSERT_GL_NO_ERROR();

        float color[4]      = {1.0f, 0.0f, 0.0f, 1.0f};
        GLint colorLocation = glGetUniformLocation(program, "u_color");
        glUniform4fv(colorLocation, 1, color);

        GLint positionLocation = glGetAttribLocation(program, "a_position");
        render(positionLocation, !mCoherentExtension);

        ASSERT_GL_NO_ERROR();

        // All fragment shaders add the input color with the uniform.  Except the COMPLEX shaders
        // which initialize attachments 0 and 2, or 1 and 3 with the uniform only (and don't use
        // input attachments for these indices).
        GLColor expect0 = GLColor::white;
        GLColor expect1 = GLColor::yellow;
        GLColor expect2 = GLColor::magenta;
        GLColor expect3 = GLColor::red;
        switch (whichFragmentShader)
        {
            case GLSL100_COMPLEX:
            case GLSL310_COMPLEX:
                if (mBothExtensions)
                {
                    expect1 = GLColor::red;
                    expect3 = GLColor::red;
                }
                else
                {
                    expect0 = GLColor::red;
                    expect2 = GLColor::red;
                }
                break;
            default:
                break;
        }

        glReadBuffer(colorAttachments[0]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, expect0);
        glReadBuffer(colorAttachments[1]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, expect1);
        glReadBuffer(colorAttachments[2]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, expect2);
        glReadBuffer(colorAttachments[3]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, expect3);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void MultipleRenderTargetArrayTest(GLProgram &program)
    {
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        std::vector<GLColor> color0(kViewportWidth * kViewportHeight, GLColor::black);
        std::vector<GLColor> color1(kViewportWidth * kViewportHeight, GLColor::green);
        std::vector<GLColor> color2(kViewportWidth * kViewportHeight, GLColor::blue);
        std::vector<GLColor> color3(kViewportWidth * kViewportHeight, GLColor::cyan);
        GLTexture colorBufferTex[kMaxColorBuffer];
        GLenum colorAttachments[kMaxColorBuffer] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                                    GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glBindTexture(GL_TEXTURE_2D, colorBufferTex[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color0.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color1.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color2.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex[3]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color3.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        for (unsigned int i = 0; i < kMaxColorBuffer; i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachments[i], GL_TEXTURE_2D,
                                   colorBufferTex[i], 0);
        }
        glDrawBuffers(kMaxColorBuffer, &colorAttachments[0]);

        ASSERT_GL_NO_ERROR();

        float color[4]      = {1.0f, 0.0f, 0.0f, 1.0f};
        GLint colorLocation = glGetUniformLocation(program, "u_color");
        glUniform4fv(colorLocation, 1, color);

        GLint positionLocation = glGetAttribLocation(program, "a_position");
        render(positionLocation, !mCoherentExtension);

        ASSERT_GL_NO_ERROR();

        glReadBuffer(colorAttachments[0]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
        glReadBuffer(colorAttachments[1]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);
        glReadBuffer(colorAttachments[2]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
        glReadBuffer(colorAttachments[3]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::white);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void MultipleDrawTest(GLProgram &program)
    {
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        std::vector<GLColor> greenColor(kViewportWidth * kViewportHeight, GLColor::green);
        GLTexture colorBufferTex;
        glBindTexture(GL_TEXTURE_2D, colorBufferTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, greenColor.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex,
                               0);

        ASSERT_GL_NO_ERROR();

        float color1[4]     = {1.0f, 0.0f, 0.0f, 1.0f};
        GLint colorLocation = glGetUniformLocation(program, "u_color");
        glUniform4fv(colorLocation, 1, color1);

        GLint positionLocation = glGetAttribLocation(program, "a_position");
        render(positionLocation, !mCoherentExtension);

        float color2[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        glUniform4fv(colorLocation, 1, color2);

        render(positionLocation, !mCoherentExtension);

        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::white);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DrawNonFetchDrawFetchTest(GLProgram &programNonFetch, GLProgram &programFetch)
    {
        glUseProgram(programNonFetch);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        std::vector<GLColor> greenColor(kViewportWidth * kViewportHeight, GLColor::green);
        GLTexture colorBufferTex;
        glBindTexture(GL_TEXTURE_2D, colorBufferTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, greenColor.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex,
                               0);

        ASSERT_GL_NO_ERROR();

        float colorRed[4]           = {1.0f, 0.0f, 0.0f, 1.0f};
        GLint colorLocationNonFetch = glGetUniformLocation(programNonFetch, "u_color");
        glUniform4fv(colorLocationNonFetch, 1, colorRed);

        GLint positionLocationNonFetch = glGetAttribLocation(programNonFetch, "a_position");
        // Render without regard to glFramebufferFetchBarrierEXT()
        render(positionLocationNonFetch, GL_FALSE);

        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        glUseProgram(programFetch);

        float colorGreen[4]      = {0.0f, 1.0f, 0.0f, 1.0f};
        GLint colorLocationFetch = glGetUniformLocation(programFetch, "u_color");
        glUniform4fv(colorLocationFetch, 1, colorGreen);

        GLint positionLocationFetch = glGetAttribLocation(programFetch, "a_position");
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocationFetch, !mCoherentExtension);

        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

        glUseProgram(programNonFetch);
        glUniform4fv(colorLocationNonFetch, 1, colorRed);
        // Render without regard to glFramebufferFetchBarrierEXT()
        render(positionLocationNonFetch, GL_FALSE);

        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        glUseProgram(programFetch);
        glUniform4fv(colorLocationFetch, 1, colorGreen);
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocationFetch, !mCoherentExtension);

        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DrawFetchDrawNonFetchTest(GLProgram &programNonFetch, GLProgram &programFetch)
    {
        glUseProgram(programFetch);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        std::vector<GLColor> greenColor(kViewportWidth * kViewportHeight, GLColor::green);
        GLTexture colorBufferTex;
        glBindTexture(GL_TEXTURE_2D, colorBufferTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, greenColor.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex,
                               0);

        ASSERT_GL_NO_ERROR();

        float colorRed[4]        = {1.0f, 0.0f, 0.0f, 1.0f};
        GLint colorLocationFetch = glGetUniformLocation(programFetch, "u_color");
        glUniform4fv(colorLocationFetch, 1, colorRed);

        GLint positionLocationFetch = glGetAttribLocation(programFetch, "a_position");
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocationFetch, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

        glUseProgram(programNonFetch);

        GLint colorLocationNonFetch = glGetUniformLocation(programNonFetch, "u_color");
        glUniform4fv(colorLocationNonFetch, 1, colorRed);

        GLint positionLocationNonFetch = glGetAttribLocation(programNonFetch, "a_position");
        // Render without regard to glFramebufferFetchBarrierEXT()
        render(positionLocationNonFetch, GL_FALSE);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        float colorGreen[4] = {0.0f, 1.0f, 0.0f, 1.0f};
        glUseProgram(programFetch);
        glUniform4fv(colorLocationFetch, 1, colorGreen);
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocationFetch, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

        glUseProgram(programNonFetch);
        glUniform4fv(colorLocationNonFetch, 1, colorRed);
        // Render without regard to glFramebufferFetchBarrierEXT()
        render(positionLocationNonFetch, GL_FALSE);

        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    enum class StorageBufferTestPostFetchAction
    {
        Nothing,
        Clear,
    };

    void DrawNonFetchDrawFetchInStorageBufferTest(GLProgram &programNonFetch,
                                                  GLProgram &programFetch,
                                                  StorageBufferTestPostFetchAction postFetchAction)
    {
        // Create output buffer
        constexpr GLsizei kBufferSize = kViewportWidth * kViewportHeight * sizeof(float[4]);
        GLBuffer buffer;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, kBufferSize, nullptr, GL_STATIC_DRAW);
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffer, 0, kBufferSize);

        // Zero-initialize it
        void *bufferData = glMapBufferRange(
            GL_SHADER_STORAGE_BUFFER, 0, kBufferSize,
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        memset(bufferData, 0, kBufferSize);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glUseProgram(programNonFetch);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        std::vector<GLColor> initColor(kViewportWidth * kViewportHeight, GLColor{10, 20, 30, 40});
        GLTexture colorBufferTex;
        glBindTexture(GL_TEXTURE_2D, colorBufferTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, initColor.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex,
                               0);

        ASSERT_GL_NO_ERROR();

        float colorRed[4]           = {1.0f, 0.0f, 0.0f, 1.0f};
        GLint colorLocationNonFetch = glGetUniformLocation(programNonFetch, "u_color");
        glUniform4fv(colorLocationNonFetch, 1, colorRed);

        GLint positionLocationNonFetch = glGetAttribLocation(programNonFetch, "a_position");

        // Mask color output.  The no-fetch draw call should be a no-op, and the fetch draw-call
        // should only output to the storage buffer, but not the color attachment.
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // Render without regard to glFramebufferFetchBarrierEXT()
        render(positionLocationNonFetch, GL_FALSE);

        ASSERT_GL_NO_ERROR();

        glUseProgram(programFetch);

        float colorBlue[4]       = {0.0f, 0.0f, 1.0f, 1.0f};
        GLint colorLocationFetch = glGetUniformLocation(programFetch, "u_color");
        glUniform4fv(colorLocationFetch, 1, colorBlue);

        GLint positionLocationFetch = glGetAttribLocation(programFetch, "a_position");
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocationFetch, !mCoherentExtension);

        ASSERT_GL_NO_ERROR();

        // Enable the color mask and clear the alpha channel.  This shouldn't be reordered with the
        // fetch draw.
        GLColor expect = initColor[0];
        if (postFetchAction == StorageBufferTestPostFetchAction::Clear)
        {
            expect.A = 200;
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
            glClearColor(0.5, 0.6, 0.7, expect.A / 255.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        // Since color is completely masked out, the texture should retain its original green color.
        EXPECT_PIXEL_COLOR_NEAR(kViewportWidth / 2, kViewportHeight / 2, expect, 1);

        // Read back the storage buffer and make sure framebuffer fetch worked as intended despite
        // masked color.
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

        const float *colorData = static_cast<const float *>(
            glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, GL_MAP_READ_BIT));
        for (uint32_t y = 0; y < kViewportHeight; ++y)
        {
            for (uint32_t x = 0; x < kViewportWidth; ++x)
            {
                uint32_t ssboIndex = (y * kViewportWidth + x) * 4;
                EXPECT_NEAR(colorData[ssboIndex + 0], initColor[0].R / 255.0, 0.05);
                EXPECT_NEAR(colorData[ssboIndex + 1], initColor[0].G / 255.0, 0.05);
                EXPECT_NEAR(colorData[ssboIndex + 2], initColor[0].B / 255.0, 0.05);
                EXPECT_NEAR(colorData[ssboIndex + 3], initColor[0].A / 255.0, 0.05);
            }
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DrawNonFetchDrawFetchWithDifferentAttachmentsTest(GLProgram &programNonFetch,
                                                           GLProgram &programFetch)
    {
        glUseProgram(programNonFetch);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        std::vector<GLColor> greenColor(kViewportWidth * kViewportHeight, GLColor::green);
        GLTexture colorTex;
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, greenColor.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

        ASSERT_GL_NO_ERROR();

        float colorRed[4]           = {1.0f, 0.0f, 0.0f, 1.0f};
        GLint colorLocationNonFetch = glGetUniformLocation(programNonFetch, "u_color");
        glUniform4fv(colorLocationNonFetch, 1, colorRed);

        GLint positionLocationNonFetch = glGetAttribLocation(programNonFetch, "a_position");
        // Render without regard to glFramebufferFetchBarrierEXT()
        render(positionLocationNonFetch, GL_FALSE);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        glUseProgram(programFetch);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer framebufferMRT1;
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferMRT1);
        std::vector<GLColor> color1(kViewportWidth * kViewportHeight, GLColor::green);
        std::vector<GLColor> color2(kViewportWidth * kViewportHeight, GLColor::blue);
        GLTexture colorBufferTex1[kMaxColorBuffer];
        GLenum colorAttachments[kMaxColorBuffer] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                                    GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glBindTexture(GL_TEXTURE_2D, colorBufferTex1[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color1.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex1[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color1.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex1[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color2.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex1[3]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color2.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        for (unsigned int i = 0; i < kMaxColorBuffer; i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachments[i], GL_TEXTURE_2D,
                                   colorBufferTex1[i], 0);
        }
        glDrawBuffers(kMaxColorBuffer, &colorAttachments[0]);
        ASSERT_GL_NO_ERROR();

        GLint colorLocation = glGetUniformLocation(programFetch, "u_color");
        glUniform4fv(colorLocation, 1, colorRed);

        GLint positionLocation = glGetAttribLocation(programFetch, "a_position");
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocation, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        glReadBuffer(colorAttachments[0]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);
        glReadBuffer(colorAttachments[1]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
        glReadBuffer(colorAttachments[2]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
        glReadBuffer(colorAttachments[3]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        GLFramebuffer framebufferMRT2;
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferMRT2);
        GLTexture colorBufferTex2[kMaxColorBuffer];
        glBindTexture(GL_TEXTURE_2D, colorBufferTex2[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color2.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex2[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color2.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex2[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color1.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex2[3]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color1.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        for (unsigned int i = 0; i < kMaxColorBuffer; i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachments[i], GL_TEXTURE_2D,
                                   colorBufferTex2[i], 0);
        }
        glDrawBuffers(kMaxColorBuffer, &colorAttachments[0]);
        ASSERT_GL_NO_ERROR();

        glUniform4fv(colorLocation, 1, colorRed);
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocation, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        glReadBuffer(colorAttachments[0]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
        glReadBuffer(colorAttachments[1]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
        glReadBuffer(colorAttachments[2]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);
        glReadBuffer(colorAttachments[3]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DrawNonFetchDrawFetchWithDifferentProgramsTest(GLProgram &programNonFetch,
                                                        GLProgram &programFetch1,
                                                        GLProgram &programFetch2)
    {
        glUseProgram(programNonFetch);
        ASSERT_GL_NO_ERROR();
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        std::vector<GLColor> greenColor(kViewportWidth * kViewportHeight, GLColor::green);
        GLTexture colorTex;
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, greenColor.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

        ASSERT_GL_NO_ERROR();

        float colorRed[4]           = {1.0f, 0.0f, 0.0f, 1.0f};
        GLint colorLocationNonFetch = glGetUniformLocation(programNonFetch, "u_color");
        glUniform4fv(colorLocationNonFetch, 1, colorRed);

        GLint positionLocationNonFetch = glGetAttribLocation(programNonFetch, "a_position");
        // Render without regard to glFramebufferFetchBarrierEXT()
        render(positionLocationNonFetch, GL_FALSE);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        glUseProgram(programFetch1);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer framebufferMRT1;
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferMRT1);
        std::vector<GLColor> color1(kViewportWidth * kViewportHeight, GLColor::green);
        GLTexture colorBufferTex1[kMaxColorBuffer];
        GLenum colorAttachments[kMaxColorBuffer] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                                    GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        for (unsigned int i = 0; i < kMaxColorBuffer; i++)
        {
            glBindTexture(GL_TEXTURE_2D, colorBufferTex1[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, color1.data());
            glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachments[i], GL_TEXTURE_2D,
                                   colorBufferTex1[i], 0);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        glDrawBuffers(kMaxColorBuffer, &colorAttachments[0]);
        ASSERT_GL_NO_ERROR();

        GLint colorLocation = glGetUniformLocation(programFetch1, "u_color");
        glUniform4fv(colorLocation, 1, colorRed);

        GLint positionLocation = glGetAttribLocation(programFetch1, "a_position");
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocation, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        glReadBuffer(colorAttachments[0]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);
        glReadBuffer(colorAttachments[1]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
        glReadBuffer(colorAttachments[2]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);
        glReadBuffer(colorAttachments[3]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        glUseProgram(programFetch2);
        ASSERT_GL_NO_ERROR();

        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        GLint colorLocation1 = glGetUniformLocation(programFetch2, "u_color");
        glUniform4fv(colorLocation1, 1, colorRed);

        GLint positionLocation1 = glGetAttribLocation(programFetch2, "a_position");
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocation1, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        glReadBuffer(colorAttachments[0]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);
        glReadBuffer(colorAttachments[1]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
        glReadBuffer(colorAttachments[2]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
        glReadBuffer(colorAttachments[3]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DrawFetchWithDifferentIndicesInSameRenderPassTest(GLProgram &programFetch1,
                                                           GLProgram &programFetch2)
    {
        GLFramebuffer framebufferMRT1;
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferMRT1);
        std::vector<GLColor> color1(kViewportWidth * kViewportHeight, GLColor::green);
        GLTexture colorBufferTex1[kMaxColorBuffer];
        GLenum colorAttachments[kMaxColorBuffer] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                                    GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        for (unsigned int i = 0; i < kMaxColorBuffer; i++)
        {
            glBindTexture(GL_TEXTURE_2D, colorBufferTex1[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, color1.data());
            glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachments[i], GL_TEXTURE_2D,
                                   colorBufferTex1[i], 0);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        glDrawBuffers(kMaxColorBuffer, &colorAttachments[0]);
        ASSERT_GL_NO_ERROR();

        glUseProgram(programFetch1);
        ASSERT_GL_NO_ERROR();

        GLint colorLocation     = glGetUniformLocation(programFetch1, "u_color");
        const float colorRed[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        glUniform4fv(colorLocation, 1, colorRed);

        GLint positionLocation = glGetAttribLocation(programFetch1, "a_position");
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        //
        // Attachments are red, yellow, yellow, red
        render(positionLocation, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        glUseProgram(programFetch2);
        ASSERT_GL_NO_ERROR();

        GLint colorLocation1     = glGetUniformLocation(programFetch2, "u_color");
        const float colorBlue[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        glUniform4fv(colorLocation1, 1, colorBlue);

        GLint positionLocation1 = glGetAttribLocation(programFetch2, "a_position");
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        //
        // Attachments are blue, blue, white, magenta
        render(positionLocation1, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        glReadBuffer(colorAttachments[0]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::blue);
        glReadBuffer(colorAttachments[1]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::blue);
        glReadBuffer(colorAttachments[2]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::white);
        glReadBuffer(colorAttachments[3]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DrawFetchBlitDrawFetchTest(GLProgram &programNonFetch, GLProgram &programFetch)
    {
        glUseProgram(programFetch);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer framebufferMRT1;
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferMRT1);
        std::vector<GLColor> color1(kViewportWidth * kViewportHeight, GLColor::green);
        std::vector<GLColor> color2(kViewportWidth * kViewportHeight, GLColor::blue);
        GLTexture colorBufferTex1[kMaxColorBuffer];
        GLenum colorAttachments[kMaxColorBuffer] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                                    GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glBindTexture(GL_TEXTURE_2D, colorBufferTex1[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color1.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex1[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color1.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex1[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color2.data());
        glBindTexture(GL_TEXTURE_2D, colorBufferTex1[3]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color2.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        for (unsigned int i = 0; i < kMaxColorBuffer; i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachments[i], GL_TEXTURE_2D,
                                   colorBufferTex1[i], 0);
        }
        glDrawBuffers(kMaxColorBuffer, &colorAttachments[0]);
        ASSERT_GL_NO_ERROR();

        float colorRed[4]   = {1.0f, 0.0f, 0.0f, 1.0f};
        GLint colorLocation = glGetUniformLocation(programFetch, "u_color");
        glUniform4fv(colorLocation, 1, colorRed);

        GLint positionLocation = glGetAttribLocation(programFetch, "a_position");
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocation, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        glReadBuffer(colorAttachments[0]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);
        glReadBuffer(colorAttachments[1]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
        glReadBuffer(colorAttachments[2]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
        glReadBuffer(colorAttachments[3]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        GLFramebuffer framebufferColor;
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferColor);

        GLTexture colorTex;
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, color2.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

        glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, framebufferColor);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, framebufferMRT1);

        glBlitFramebuffer(0, 0, kViewportWidth, kViewportHeight, 0, 0, kViewportWidth,
                          kViewportHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, framebufferMRT1);
        glReadBuffer(colorAttachments[0]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::blue);
        glReadBuffer(colorAttachments[1]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::blue);
        glReadBuffer(colorAttachments[2]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::blue);
        glReadBuffer(colorAttachments[3]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::blue);

        float colorGreen[4] = {0.0f, 1.0f, 0.0f, 1.0f};
        glUniform4fv(colorLocation, 1, colorGreen);

        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocation, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        glReadBuffer(colorAttachments[0]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::cyan);
        glReadBuffer(colorAttachments[1]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::green);
        glReadBuffer(colorAttachments[2]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::cyan);
        glReadBuffer(colorAttachments[3]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::green);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void makeProgramPipeline(GLProgramPipeline &pipeline, const char *vs, const char *fs)
    {
        GLProgram programVS, programFS;

        GLShader vertShader(GL_VERTEX_SHADER);
        glShaderSource(vertShader, 1, &vs, nullptr);
        glCompileShader(vertShader);
        glProgramParameteri(programVS, GL_PROGRAM_SEPARABLE, GL_TRUE);
        glAttachShader(programVS, vertShader);
        glLinkProgram(programVS);
        ASSERT_GL_NO_ERROR();

        GLShader fragShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, &fs, nullptr);
        glCompileShader(fragShader);
        glProgramParameteri(programFS, GL_PROGRAM_SEPARABLE, GL_TRUE);
        glAttachShader(programFS, fragShader);
        glLinkProgram(programFS);
        ASSERT_GL_NO_ERROR();

        glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, programVS);
        glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, programFS);

        glUseProgram(0);
        glBindProgramPipeline(pipeline);
        ASSERT_GL_NO_ERROR();
    }

    void ProgramPipelineTest(const char *kVS, const char *kFS1, const char *kFS2)
    {
        GLProgram programVert, programNonFetch, programFetch;
        const char *sourceArray[3] = {kVS, kFS1, kFS2};

        GLShader vertShader(GL_VERTEX_SHADER);
        glShaderSource(vertShader, 1, &sourceArray[0], nullptr);
        glCompileShader(vertShader);
        glProgramParameteri(programVert, GL_PROGRAM_SEPARABLE, GL_TRUE);
        glAttachShader(programVert, vertShader);
        glLinkProgram(programVert);
        ASSERT_GL_NO_ERROR();

        GLShader fragShader1(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader1, 1, &sourceArray[1], nullptr);
        glCompileShader(fragShader1);
        glProgramParameteri(programNonFetch, GL_PROGRAM_SEPARABLE, GL_TRUE);
        glAttachShader(programNonFetch, fragShader1);
        glLinkProgram(programNonFetch);
        ASSERT_GL_NO_ERROR();

        GLShader fragShader2(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader2, 1, &sourceArray[2], nullptr);
        glCompileShader(fragShader2);
        glProgramParameteri(programFetch, GL_PROGRAM_SEPARABLE, GL_TRUE);
        glAttachShader(programFetch, fragShader2);
        glLinkProgram(programFetch);
        ASSERT_GL_NO_ERROR();

        GLProgramPipeline pipeline1, pipeline2, pipeline3, pipeline4;
        glUseProgramStages(pipeline1, GL_VERTEX_SHADER_BIT, programVert);
        glUseProgramStages(pipeline1, GL_FRAGMENT_SHADER_BIT, programNonFetch);
        glBindProgramPipeline(pipeline1);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        std::vector<GLColor> greenColor(kViewportWidth * kViewportHeight, GLColor::green);
        GLTexture colorBufferTex;
        glBindTexture(GL_TEXTURE_2D, colorBufferTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, greenColor.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex,
                               0);
        ASSERT_GL_NO_ERROR();

        glActiveShaderProgram(pipeline1, programNonFetch);
        float colorRed[4]           = {1.0f, 0.0f, 0.0f, 1.0f};
        GLint colorLocationNonFetch = glGetUniformLocation(programNonFetch, "u_color");
        glUniform4fv(colorLocationNonFetch, 1, colorRed);
        ASSERT_GL_NO_ERROR();

        glActiveShaderProgram(pipeline1, programVert);
        GLint positionLocation = glGetAttribLocation(programVert, "a_position");
        // Render without regard to glFramebufferFetchBarrierEXT()
        render(positionLocation, GL_FALSE);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        glUseProgramStages(pipeline2, GL_VERTEX_SHADER_BIT, programVert);
        glUseProgramStages(pipeline2, GL_FRAGMENT_SHADER_BIT, programFetch);
        glBindProgramPipeline(pipeline2);
        ASSERT_GL_NO_ERROR();

        glActiveShaderProgram(pipeline2, programFetch);
        float colorGreen[4]      = {0.0f, 1.0f, 0.0f, 1.0f};
        GLint colorLocationFetch = glGetUniformLocation(programFetch, "u_color");
        glUniform4fv(colorLocationFetch, 1, colorGreen);

        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocation, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

        glUseProgramStages(pipeline3, GL_VERTEX_SHADER_BIT, programVert);
        glUseProgramStages(pipeline3, GL_FRAGMENT_SHADER_BIT, programNonFetch);
        glBindProgramPipeline(pipeline3);
        ASSERT_GL_NO_ERROR();

        glActiveShaderProgram(pipeline3, programNonFetch);
        colorLocationNonFetch = glGetUniformLocation(programNonFetch, "u_color");
        glUniform4fv(colorLocationNonFetch, 1, colorRed);

        ASSERT_GL_NO_ERROR();

        // Render without regard to glFramebufferFetchBarrierEXT()
        render(positionLocation, GL_FALSE);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);

        glUseProgramStages(pipeline4, GL_VERTEX_SHADER_BIT, programVert);
        glUseProgramStages(pipeline4, GL_FRAGMENT_SHADER_BIT, programFetch);
        glBindProgramPipeline(pipeline4);
        ASSERT_GL_NO_ERROR();

        glActiveShaderProgram(pipeline4, programFetch);
        colorLocationFetch = glGetUniformLocation(programFetch, "u_color");
        glUniform4fv(colorLocationFetch, 1, colorGreen);
        // Render potentially with a glFramebufferFetchBarrierEXT() depending on the [non-]coherent
        // extension being used
        render(positionLocation, !mCoherentExtension);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void framebufferFetchDepthStencilDetachSeparately(GLenum depthStencilFormat)
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

        const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

layout(location=0) out highp vec4 color;

highp float gl_LastFragDepthARM;
highp int gl_LastFragStencilARM;

void main()
{
    color = vec4(float(gl_LastFragStencilARM)/255.0, gl_LastFragDepthARM, 0, 1);
})";

        GLRenderbuffer color[4], depthStencil;
        GLFramebuffer fbo;

        stateReset();
        // Create FBO with depth/stencil
        createFboWithDepthStencilAndMRT(1, 1, 0, depthStencilFormat, &fbo, color, &depthStencil);
        ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
        ASSERT_GL_NO_ERROR();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        glClearDepthf(0.8f);
        glClearStencil(0x3C);
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        glUseProgram(program);
        glStencilFunc(GL_LESS, 40, 0xFF);
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.4f);
        EXPECT_PIXEL_RECT_EQ(0, 0, 1, 1, GLColor(60, 204, 0, 255));

        // CASE 1: Detach stencil, depth is still attached
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        ASSERT_GL_NO_ERROR();
        glUseProgram(program);
        glStencilFunc(GL_LESS, 30, 0xFF);
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.3f);
        ASSERT_GL_NO_ERROR();
        GLColor actual0;
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &actual0.R);
        EXPECT_EQ(178, actual0.G);

        // CASE 2: Detach depth and attach old stencil
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  depthStencil);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        ASSERT_GL_NO_ERROR();
        glUseProgram(program);
        glStencilFunc(GL_LESS, 20, 0xFF);
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.2f);
        ASSERT_GL_NO_ERROR();
        GLColor actual1;
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &actual1.R);
        EXPECT_EQ(40, actual1.R);

        // CASE 3: Attach old depth
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                  depthStencil);
        glUseProgram(program);
        glStencilFunc(GL_LESS, 10, 0xFF);
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.1f);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_RECT_EQ(0, 0, 1, 1, GLColor(20, 166, 0, 255));
    }

    const char *getFragShaderName(GLenum depthStencilFormat)
    {
        static const char kFS1[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp float gl_LastFragDepthARM;
highp int gl_LastFragStencilARM;

layout(location=0) out highp vec4 fragColor0;
layout(location=1) out highp vec4 fragColor1;
layout(location=2) out highp vec4 fragColor2;
layout(location=3) out highp vec4 fragColor3;

void main()
{
    fragColor0 = fragColor1 = fragColor2 = fragColor3 = vec4(gl_LastFragDepthARM,
    float(gl_LastFragStencilARM)/255.0, gl_FragCoord.z, 1.0);
})";

        static const char kFS2[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp float gl_LastFragDepthARM;

layout(location=0) out highp vec4 fragColor0;
layout(location=1) out highp vec4 fragColor1;
layout(location=2) out highp vec4 fragColor2;
layout(location=3) out highp vec4 fragColor3;

void main()
{
    fragColor0 = fragColor1 = fragColor2 = fragColor3 = vec4(gl_LastFragDepthARM, 0.0,
    gl_FragCoord.z, 1.0);
})";

        static const char kFS3[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp int gl_LastFragStencilARM;

layout(location=0) out highp vec4 fragColor0;
layout(location=1) out highp vec4 fragColor1;
layout(location=2) out highp vec4 fragColor2;
layout(location=3) out highp vec4 fragColor3;

void main()
{
    fragColor0 = fragColor1 = fragColor2 = fragColor3 = vec4(0.0,
    float(gl_LastFragStencilARM)/255.0, gl_FragCoord.z, 1.0);
})";

        bool depth   = depthFormatBitCount(depthStencilFormat) > 0;
        bool stencil = stencilFormatBitCount(depthStencilFormat) > 0;

        if (depth && stencil)
        {
            return kFS1;
        }
        else if (depth)
        {
            return kFS2;
        }
        else
        {
            return kFS3;
        }
    }

    void stateReset()
    {
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_FETCH_PER_SAMPLE_ARM);

        glBlendFunc(GL_ONE, GL_ZERO);
        glDepthFunc(GL_LEQUAL);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_ALWAYS, 0, 0xFFFFu);

        glClearDepthf(1.f);
        glClearStencil(0);
        glClearColor(0.f, 0.f, 0.f, 0.f);
    }

    void createFboWithDepthStencilAndMRT(int width,
                                         int height,
                                         int samples,
                                         GLenum depthStencilFormat,
                                         GLFramebuffer *fbo,
                                         GLRenderbuffer *color,
                                         GLRenderbuffer *depthStencil)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
        ASSERT_GL_NO_ERROR();
        for (GLuint i = 0; i < kMaxColorBuffer; ++i)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, color[i]);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER,
                                      color[i]);
            ASSERT_GL_NO_ERROR();
        }
        glBindRenderbuffer(GL_RENDERBUFFER, *depthStencil);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, depthStencilFormat, width,
                                         height);
        if (depthFormatBitCount(depthStencilFormat) > 0)
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                      *depthStencil);
        }
        if (stencilFormatBitCount(depthStencilFormat) > 0)
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                      *depthStencil);
        }
        ASSERT_GL_NO_ERROR();
        GLenum drawBuffers[kMaxColorBuffer] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                               GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(kMaxColorBuffer, drawBuffers);
        glViewport(0, 0, width, height);
        ASSERT_GL_NO_ERROR();
    }
    GLColor filterDepthStencilColor(GLColor color, GLenum depthStencilFormat)
    {
        if (depthFormatBitCount(depthStencilFormat) == 0)
        {
            color.R = 0;
        }
        if (stencilFormatBitCount(depthStencilFormat) == 0)
        {
            color.G = 0;
        }
        return color;
    }

    int depthFormatBitCount(GLenum format)
    {
        switch (format)
        {
            case GL_DEPTH_COMPONENT16:
                return 16;

            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH24_STENCIL8:
                return 24;

            case GL_DEPTH32F_STENCIL8:
            case GL_DEPTH_COMPONENT32F:
                return 32;

            default:
                return 0;
        }
    }

    int stencilFormatBitCount(GLenum format)
    {
        switch (format)
        {
            case GL_DEPTH24_STENCIL8:
            case GL_DEPTH32F_STENCIL8:
            case GL_STENCIL_INDEX8:
                return 8;

            default:
                return 0;
        }
    }

    int maxSamplesSupported(GLenum format)
    {
        GLint samples = 0;
        glGetInternalformativ(GL_RENDERBUFFER, format, GL_SAMPLES, 1, &samples);
        return samples;
    }

    bool sampleCountSupported(GLenum target, GLenum format, int sampleCount)
    {
        GLint numSupportedSampleCounts = 0;
        GLint supportedSampleCounts[8] = {0};
        glGetInternalformativ(target, format, GL_NUM_SAMPLE_COUNTS, 1, &numSupportedSampleCounts);
        glGetInternalformativ(target, format, GL_SAMPLES, numSupportedSampleCounts,
                              supportedSampleCounts);
        for (int i = 0; i < numSupportedSampleCounts; ++i)
        {
            if (supportedSampleCounts[i] == sampleCount || sampleCount == 0)
            {
                return true;
            }
        }
        return false;
    }

    void bindResolveFboAndVerify(GLRenderbuffer *resolve,
                                 GLFramebuffer *resolveFbo,
                                 GLsizei width,
                                 GLsizei height,
                                 bool isBlit,
                                 bool isDiscard,
                                 GLFramebuffer *fbo,
                                 GLenum depthStencilFormat)
    {
        if (!isBlit)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, *resolve);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
            glBindFramebuffer(GL_FRAMEBUFFER, *resolveFbo);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      *resolve);
            ASSERT_GL_NO_ERROR();
        }
        else
        {
            for (GLuint index = 0; index < kMaxColorBuffer; ++index)
            {
                glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *resolveFbo);
                glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT,
                                  GL_NEAREST);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, *resolveFbo);
                ASSERT_GL_NO_ERROR();
                if (!isDiscard)
                {
                    EXPECT_PIXEL_RECT_EQ(
                        0, 0, width, height,
                        filterDepthStencilColor(GLColor(255, 70, 191, 255), depthStencilFormat));
                }
                else
                {
                    for (GLsizei x = 0; x < width; ++x)
                    {
                        for (GLsizei y = 0; y < height; ++y)
                        {
                            if ((x + y) % 2 != 0)
                            {
                                EXPECT_PIXEL_COLOR_EQ(x, y, GLColor::blue);
                            }
                            else
                            {
                                EXPECT_PIXEL_COLOR_EQ(x, y, GLColor::white);
                            }
                        }
                    }
                }
                glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
            }
        }
    }

    void clearAndDrawQuad(GLuint program, bool isDiscard)
    {
        glClearDepthf(1.f);
        glClearStencil(70);
        glClearColor(1.f, 1.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        if (!isDiscard)
        {
            glUseProgram(program);
            drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
            ASSERT_GL_NO_ERROR();
        }
        else
        {
            glUseProgram(program);
            GLint colorLocation = glGetUniformLocation(program, "color");
            glUniform4fv(colorLocation, 1, GLColor::blue.toNormalizedVector().data());
            drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
            ASSERT_GL_NO_ERROR();
        }
    }

    void createFramebufferWithDepthStencil(GLRenderbuffer *color,
                                           GLRenderbuffer *depthStencil,
                                           GLFramebuffer *fbo);

    // Helpers for tests that don't care whether coherent or non-coherent framebuffer fetch is
    // enabled, because they are testing something orthogonal to coherence.  They only account for
    // GL_EXT_shader_framebuffer_fetch and GL_EXT_shader_framebuffer_fetch_non_coherent, not the ARM
    // variant or depth/stencil.
    WhichExtension chooseBetweenCoherentOrIncoherent();
    std::string makeShaderPreamble(WhichExtension whichExtension,
                                   const char *otherExtensions,
                                   uint32_t colorAttachmentCount);

    bool mCoherentExtension;
    bool mARMExtension;
    bool mBothExtensions;
};

class FramebufferFetchAndAdvancedBlendES31 : public FramebufferFetchES31
{};

void FramebufferFetchES31::createFramebufferWithDepthStencil(GLRenderbuffer *color,
                                                             GLRenderbuffer *depthStencil,
                                                             GLFramebuffer *fbo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

    glBindRenderbuffer(GL_RENDERBUFFER, *color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *color);

    glBindRenderbuffer(GL_RENDERBUFFER, *depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kViewportWidth, kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              *depthStencil);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();
}

FramebufferFetchES31::WhichExtension FramebufferFetchES31::chooseBetweenCoherentOrIncoherent()
{
    const bool isCoherent = IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch");
    EXPECT_TRUE(isCoherent || IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    return isCoherent ? COHERENT : NON_COHERENT;
}

std::string FramebufferFetchES31::makeShaderPreamble(WhichExtension whichExtension,
                                                     const char *otherExtensions,
                                                     uint32_t colorAttachmentCount)
{
    std::ostringstream fs;
    fs << "#version 310 es\n";
    switch (whichExtension)
    {
        case COHERENT:
            fs << "#extension GL_EXT_shader_framebuffer_fetch : require\n";
            break;
        case NON_COHERENT:
            fs << "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n";
            break;
        default:
            UNREACHABLE();
            break;
    }

    if (otherExtensions != nullptr)
    {
        fs << otherExtensions << "\n";
    }

    for (uint32_t location = 0; location < colorAttachmentCount; ++location)
    {
        fs << "layout(";
        if (whichExtension == NON_COHERENT)
        {
            fs << "noncoherent, ";
        }
        fs << "location = " << location << ") inout highp vec4 color" << location << ";\n";
    }

    return fs.str();
}

// Test coherent extension with inout qualifier
TEST_P(FramebufferFetchES31, BasicInout_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    BasicTest(program);
}

// Test non-coherent extension with inout qualifier
TEST_P(FramebufferFetchES31, BasicInout_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    BasicTest(program);
}

// Test coherent extension with gl_LastFragData
TEST_P(FramebufferFetchES31, BasicLastFragData_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram program;
    program.makeRaster(k100VS, getFragmentShader(GLSL100));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    BasicTest(program);
}

// Test non-coherent extension with gl_LastFragData
TEST_P(FramebufferFetchES31, BasicLastFragData_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram program;
    program.makeRaster(k100VS, getFragmentShader(GLSL100));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    BasicTest(program);
}

// Testing coherent extension with multiple render target, using gl_FragData with constant indices
TEST_P(FramebufferFetchES31, MultipleRenderTarget_Coherent_FragData)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_draw_buffers"));
    setWhichExtension(COHERENT);

    GLProgram program;
    program.makeRaster(k100VS, getFragmentShader(GLSL100_4ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL100_4ATTACHMENT);
}

// Testing coherent extension with multiple render target, using gl_FragData with complex
// expressions
TEST_P(FramebufferFetchES31, MultipleRenderTarget_Coherent_FragData_Complex)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_draw_buffers"));
    setWhichExtension(COHERENT);

    GLProgram program;
    program.makeRaster(k100VS, getFragmentShader(GLSL100_COMPLEX));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL100_COMPLEX);
}

// Testing coherent extension with multiple render target, using inouts with complex expressions
TEST_P(FramebufferFetchES31, MultipleRenderTarget_Coherent_Complex)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_COMPLEX));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL310_COMPLEX);
}

// Testing coherent extension with multiple render target
TEST_P(FramebufferFetchES31, MultipleRenderTarget_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL310_4ATTACHMENT);
}

// Testing non-coherent extension with multiple render target, using gl_FragData with constant
// indices
TEST_P(FramebufferFetchES31, MultipleRenderTarget_NonCoherent_FragData)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_draw_buffers"));
    setWhichExtension(NON_COHERENT);

    GLProgram program;
    program.makeRaster(k100VS, getFragmentShader(GLSL100_4ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL100_4ATTACHMENT);
}

// Testing non-coherent extension with multiple render target, using gl_FragData with complex
// expressions
TEST_P(FramebufferFetchES31, MultipleRenderTarget_NonCoherent_FragData_Complex)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_draw_buffers"));
    setWhichExtension(NON_COHERENT);

    GLProgram program;
    program.makeRaster(k100VS, getFragmentShader(GLSL100_COMPLEX));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL100_COMPLEX);
}

// Testing non-coherent extension with multiple render target, using inouts with complex expressions
TEST_P(FramebufferFetchES31, MultipleRenderTarget_NonCoherent_Complex)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_COMPLEX));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL310_COMPLEX);
}

// Testing non-coherent extension with multiple render target
TEST_P(FramebufferFetchES31, MultipleRenderTarget_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL310_4ATTACHMENT);
}

// Testing non-coherent extension with multiple render target using inout array
TEST_P(FramebufferFetchES31, MultipleRenderTargetWithInoutArray_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL310_4ATTACHMENT);
}

// Testing coherent extension with multiple render target using inout array
TEST_P(FramebufferFetchES31, MultipleRenderTargetWithInoutArray_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL310_4ATTACHMENT);
}

// Test coherent extension with multiple draw
TEST_P(FramebufferFetchES31, MultipleDraw_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleDrawTest(program);
}

// Test non-coherent extension with multiple draw
TEST_P(FramebufferFetchES31, MultipleDraw_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleDrawTest(program);
}

// Testing coherent extension with the order of non-fetch program and fetch program
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetch_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchTest(programNonFetch, programFetch);
}

// Testing non-coherent extension with the order of non-fetch program and fetch program
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetch_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchTest(programNonFetch, programFetch);
}

// Testing coherent extension with the order of fetch program and non-fetch program
TEST_P(FramebufferFetchES31, DrawFetchDrawNonFetch_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    ASSERT_GL_NO_ERROR();

    DrawFetchDrawNonFetchTest(programNonFetch, programFetch);
}

// Testing non-coherent extension with the order of fetch program and non-fetch program
TEST_P(FramebufferFetchES31, DrawFetchDrawNonFetch_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    ASSERT_GL_NO_ERROR();

    DrawFetchDrawNonFetchTest(programNonFetch, programFetch);
}

// Testing coherent extension with framebuffer fetch read in combination with color attachment mask
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetchInStorageBuffer_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLint maxFragmentShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxFragmentShaderStorageBlocks == 0);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT_WITH_STORAGE_BUFFER));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchInStorageBufferTest(programNonFetch, programFetch,
                                             StorageBufferTestPostFetchAction::Nothing);
}

// Testing non-coherent extension with framebuffer fetch read in combination with color attachment
// mask
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetchInStorageBuffer_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLint maxFragmentShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxFragmentShaderStorageBlocks == 0);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT_WITH_STORAGE_BUFFER));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchInStorageBufferTest(programNonFetch, programFetch,
                                             StorageBufferTestPostFetchAction::Nothing);
}

// Testing coherent extension with the order of non-fetch program and fetch program with
// different attachments
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetchWithDifferentAttachments_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT1));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchWithDifferentAttachmentsTest(programNonFetch, programFetch);
}

// Testing coherent extension with framebuffer fetch read in combination with color attachment mask
// and clear
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetchInStorageBufferThenClear_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLint maxFragmentShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxFragmentShaderStorageBlocks == 0);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT_WITH_STORAGE_BUFFER));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchInStorageBufferTest(programNonFetch, programFetch,
                                             StorageBufferTestPostFetchAction::Clear);
}

// Testing non-coherent extension with framebuffer fetch read in combination with color attachment
// mask and clear
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetchInStorageBufferThenClear_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLint maxFragmentShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxFragmentShaderStorageBlocks == 0);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT_WITH_STORAGE_BUFFER));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchInStorageBufferTest(programNonFetch, programFetch,
                                             StorageBufferTestPostFetchAction::Clear);
}

// Testing non-coherent extension with the order of non-fetch program and fetch program with
// different attachments
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetchWithDifferentAttachments_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT1));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchWithDifferentAttachmentsTest(programNonFetch, programFetch);
}

// Testing coherent extension with the order of non-fetch program and fetch with different
// programs
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetchWithDifferentPrograms_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram programNonFetch, programFetch1, programFetch2;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch1.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT1));
    programFetch2.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT2));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchWithDifferentProgramsTest(programNonFetch, programFetch1, programFetch2);
}

// Testing non-coherent extension with the order of non-fetch program and fetch with different
// programs
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetchWithDifferentPrograms_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram programNonFetch, programFetch1, programFetch2;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch1.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT1));
    programFetch2.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT2));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchWithDifferentProgramsTest(programNonFetch, programFetch1, programFetch2);
}

// Testing coherent extension with two fetch programs using different attachments.  The different
// sets of attachments start at different non-zero indices.
TEST_P(FramebufferFetchES31, DrawFetchWithDifferentIndicesInSameRenderPass_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram programFetch1, programFetch2;
    programFetch1.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT3));
    programFetch2.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT4));
    ASSERT_GL_NO_ERROR();

    DrawFetchWithDifferentIndicesInSameRenderPassTest(programFetch1, programFetch2);
}

// Testing non-coherent extension with two fetch programs using different attachments.  The
// different sets of attachments start at different non-zero indices.
TEST_P(FramebufferFetchES31, DrawFetchWithDifferentIndicesInSameRenderPass_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram programFetch1, programFetch2;
    programFetch1.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT3));
    programFetch2.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT4));
    ASSERT_GL_NO_ERROR();

    DrawFetchWithDifferentIndicesInSameRenderPassTest(programFetch1, programFetch2);
}

// Testing coherent extension with the order of draw fetch, blit and draw fetch
TEST_P(FramebufferFetchES31, DrawFetchBlitDrawFetch_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT1));
    ASSERT_GL_NO_ERROR();

    DrawFetchBlitDrawFetchTest(programNonFetch, programFetch);
}

// Testing non-coherent extension with the order of draw fetch, blit and draw fetch
TEST_P(FramebufferFetchES31, DrawFetchBlitDrawFetch_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_4ATTACHMENT_DIFFERENT1));
    ASSERT_GL_NO_ERROR();

    DrawFetchBlitDrawFetchTest(programNonFetch, programFetch);
}

// Testing coherent extension with program pipeline
TEST_P(FramebufferFetchES31, ProgramPipeline_Coherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    ProgramPipelineTest(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT),
                        getFragmentShader(GLSL310_1ATTACHMENT));
}

// Testing non-coherent extension with program pipeline
TEST_P(FramebufferFetchES31, ProgramPipeline_NonCoherent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    setWhichExtension(NON_COHERENT);

    ProgramPipelineTest(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT),
                        getFragmentShader(GLSL310_1ATTACHMENT));
}

// Verify that sample shading is automatically enabled when framebuffer fetch is used with
// multisampling.
TEST_P(FramebufferFetchES31, MultiSampled)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch") &&
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_variables"));

    const WhichExtension whichExtension = chooseBetweenCoherentOrIncoherent();

    // Create a single-sampled framebuffer as the resolve target
    GLRenderbuffer resolve;
    glBindRenderbuffer(GL_RENDERBUFFER, resolve);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    GLFramebuffer resolveFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, resolve);

    // Create a multisampled framebuffer
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kViewportWidth, kViewportHeight);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    // Initialize every sample differently with per-sample shading.
    constexpr char kPrimeFS[] = R"(#version 310 es
#extension GL_OES_sample_variables : require
out highp vec4 color;
void main (void)
{
    switch (gl_SampleID)
    {
    case 0:
        color = vec4(1.0, 0.9, 0.8, 0.7);
        break;
    case 1:
        color = vec4(0.0, 0.1, 0.2, 0.3);
        break;
    case 2:
        color = vec4(0.5, 0.25, 0.75, 1.0);
        break;
    default:
        color = vec4(0.4, 0.6, 0.2, 0.8);
        break;
    }
})";
    ANGLE_GL_PROGRAM(prime, essl31_shaders::vs::Passthrough(), kPrimeFS);
    glViewport(0, 0, kViewportWidth, kViewportHeight);
    drawQuad(prime, essl31_shaders::PositionAttrib(), 0.0f);

    // Break the render pass to make sure sample shading is not left enabled by accident.
    // The expected value is the average of the values set by the shader.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFbo);
    glBlitFramebuffer(0, 0, kViewportWidth, kViewportHeight, 0, 0, kViewportWidth, kViewportHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFbo);
    EXPECT_PIXEL_NEAR(0, 0, 121, 118, 124, 178, 1);
    ASSERT_GL_NO_ERROR();

    // Use framebuffer fetch to read the value of each sample, and store the square of that value.
    // Because square is non-linear, applied to the average value it would produce a different
    // result compared with it being applied to individual samples and then averaged.  The test thus
    // ensures that framebuffer fetch on a multisampled framebuffer implicitly enables sample
    // shading.
    std::ostringstream fs;
    fs << makeShaderPreamble(whichExtension, nullptr, 1);
    fs << R"(void main()
{
    color0 *= color0;
})";

    ANGLE_GL_PROGRAM(square, essl31_shaders::vs::Passthrough(), fs.str().c_str());
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    drawQuad(square, essl31_shaders::PositionAttrib(), 0.0f);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFbo);
    glBlitFramebuffer(0, 0, kViewportWidth, kViewportHeight, 0, 0, kViewportWidth, kViewportHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFbo);

    // Verify that the result is average(square(samples)) and not square(average(samples)).
    EXPECT_PIXEL_NEAR(0, 0, 90, 79, 82, 141, 1);

    // For debugging purposes, the following would be true if framebuffer fetch _didn't_ implicitly
    // enable sample shading.
    // EXPECT_PIXEL_NEAR(0, 0, 57, 54, 60, 125, 1);

    ASSERT_GL_NO_ERROR();
}

// Test recovering a supposedly closed render pass that used framebuffer fetch.
TEST_P(FramebufferFetchES31, ReopenRenderPass)
{
    const bool isCoherent = IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch");
    ANGLE_SKIP_TEST_IF(!isCoherent &&
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    // Create two framebuffers
    GLRenderbuffer color[2];
    GLFramebuffer fbo[2];
    for (uint32_t i = 0; i < 2; ++i)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, color[i]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color[i]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Use a framebuffer fetch program.
    std::ostringstream fs;
    fs << "#version 310 es\n";
    if (isCoherent)
    {
        fs << "#extension GL_EXT_shader_framebuffer_fetch : require\n";
    }
    else
    {
        fs << "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n";
        fs << "layout(noncoherent) ";
    }
    fs << R"(inout highp vec4 color;
void main (void)
{
    color += vec4(0.25, 0.125, 0.5, 0.0);
})";

    ANGLE_GL_PROGRAM(ff, essl31_shaders::vs::Passthrough(), fs.str().c_str());
    drawQuad(ff, essl31_shaders::PositionAttrib(), 0.0f);

    // Switch to another framebuffer and do a clear.  In the Vulkan backend, the previous render
    // pass stays around.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Switch back to the original framebuffer and do a non-framebuffer fetch draw
    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
    glEnable(GL_SCISSOR_TEST);
    glScissor(kViewportWidth / 2, kViewportHeight / 2, kViewportWidth - kViewportWidth / 2,
              kViewportHeight - kViewportHeight / 2);
    drawQuad(drawRed, essl31_shaders::PositionAttrib(), 0.0f);

    // Verify the results
    EXPECT_PIXEL_NEAR(0, 0, 191, 159, 255, 255, 1);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth - 1, kViewportHeight - 1, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Test opening a render pass with a scissored clear
TEST_P(FramebufferFetchES31, StartWithScissoredClear)
{
    const bool isCoherent = IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch");
    ANGLE_SKIP_TEST_IF(!isCoherent &&
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    std::ostringstream fs;
    fs << "#version 310 es\n";
    if (isCoherent)
    {
        fs << "#extension GL_EXT_shader_framebuffer_fetch : require\n";
    }
    else
    {
        fs << "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n";
        fs << "layout(noncoherent) ";
    }
    fs << R"(inout highp vec4 color;
void main (void)
{
    color += vec4(0.25, 0.125, 0.5, 0.0);
})";

    ANGLE_GL_PROGRAM(ff, kVS, fs.str().c_str());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer color;
    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);

    // Draw once for the program to be processed, so the draw after clear would not have executable
    // related dirty bits.
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(ff);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Clear value (0.5, 0.5, 0.5, 1.0f) + shader's addition (0.25, 0.125, 0.5, 0.0)
    EXPECT_PIXEL_NEAR(0, 0, 191, 159, 255, 255, 1);

    // Start rendering with a scissored clear, then do a draw call
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kViewportWidth / 2, kViewportHeight / 2);

    glClearColor(0.125f, 0.75f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!isCoherent)
    {
        glFramebufferFetchBarrierEXT();
    }

    // Don't use drawQuad, as it reinstalls the program, adding additional dirty bits.
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Verify the results
    // Clear value (0.125, 0.75, 0.25, 1.0f) + shader's addition (0.25, 0.125, 0.5, 0.0)
    EXPECT_PIXEL_NEAR(0, 0, 96, 223, 191, 255, 1);
    // The rest of the image should be left untouched due to scissor
    EXPECT_PIXEL_NEAR(kViewportWidth - 1, kViewportHeight - 1, 191, 159, 255, 255, 1);
    ASSERT_GL_NO_ERROR();
}

// Test opening a render pass with a masked clear
TEST_P(FramebufferFetchES31, StartWithMaskedClear)
{
    const bool isCoherent = IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch");
    ANGLE_SKIP_TEST_IF(!isCoherent &&
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    std::ostringstream fs;
    fs << "#version 310 es\n";
    if (isCoherent)
    {
        fs << "#extension GL_EXT_shader_framebuffer_fetch : require\n";
    }
    else
    {
        fs << "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n";
        fs << "layout(noncoherent) ";
    }
    fs << R"(inout highp vec4 color;
void main (void)
{
    color += vec4(0.25, 0.125, 0.5, 0.0);
})";

    ANGLE_GL_PROGRAM(ff, kVS, fs.str().c_str());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer color;
    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);

    // Draw once for the program to be processed, so the draw after clear would not have executable
    // related dirty bits.
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(ff);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Clear value (0.5, 0.5, 0.5, 1.0f) + shader's addition (0.25, 0.125, 0.5, 0.0)
    EXPECT_PIXEL_NEAR(0, 0, 191, 159, 255, 255, 1);

    // Start rendering with a scissored clear, then do a draw call
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kViewportWidth / 2, kViewportHeight / 2);

    glClearColor(0.125f, 0.75f, 0.25f, 1.0f);
    glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    if (!isCoherent)
    {
        glFramebufferFetchBarrierEXT();
    }

    // Don't use drawQuad, as it reinstalls the program, adding additional dirty bits.
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Verify the results
    // Clear value (0.125, 0.75, 0.25, 1.0f) + shader's addition (0.25, 0.125, 0.5, 0.0)
    EXPECT_PIXEL_NEAR(0, 0, 96, 191, 255, 255, 1);
    // The rest of the image should be left untouched due to scissor
    EXPECT_PIXEL_NEAR(kViewportWidth - 1, kViewportHeight - 1, 191, 159, 255, 255, 1);
    ASSERT_GL_NO_ERROR();
}

// Test combination of inout and samplers.
TEST_P(FramebufferFetchES31, UniformUsageCombinations)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    constexpr char kVS[] = R"(#version 310 es
in highp vec4 a_position;
out highp vec2 texCoord;

void main()
{
    gl_Position = a_position;
    texCoord = (a_position.xy * 0.5) + 0.5;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require

layout(binding=0, offset=0) uniform atomic_uint atDiff;
uniform sampler2D tex;

layout(noncoherent, location = 0) inout highp vec4 o_color[4];
in highp vec2 texCoord;

void main()
{
    highp vec4 texColor = texture(tex, texCoord);

    if (texColor != o_color[0])
    {
        atomicCounterIncrement(atDiff);
        o_color[0] = texColor;
    }
    else
    {
        if (atomicCounter(atDiff) > 0u)
        {
            atomicCounterDecrement(atDiff);
        }
    }

    if (texColor != o_color[1])
    {
        atomicCounterIncrement(atDiff);
        o_color[1] = texColor;
    }
    else
    {
        if (atomicCounter(atDiff) > 0u)
        {
            atomicCounterDecrement(atDiff);
        }
    }

    if (texColor != o_color[2])
    {
        atomicCounterIncrement(atDiff);
        o_color[2] = texColor;
    }
    else
    {
        if (atomicCounter(atDiff) > 0u)
        {
            atomicCounterDecrement(atDiff);
        }
    }

    if (texColor != o_color[3])
    {
        atomicCounterIncrement(atDiff);
        o_color[3] = texColor;
    }
    else
    {
        if (atomicCounter(atDiff) > 0u)
        {
            atomicCounterDecrement(atDiff);
        }
    }
})";

    GLProgram program;
    program.makeRaster(kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    std::vector<GLColor> color0(kViewportWidth * kViewportHeight, GLColor::cyan);
    std::vector<GLColor> color1(kViewportWidth * kViewportHeight, GLColor::green);
    std::vector<GLColor> color2(kViewportWidth * kViewportHeight, GLColor::blue);
    std::vector<GLColor> color3(kViewportWidth * kViewportHeight, GLColor::black);
    GLTexture colorBufferTex[kMaxColorBuffer];
    GLenum colorAttachments[kMaxColorBuffer] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                                GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glBindTexture(GL_TEXTURE_2D, colorBufferTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, color0.data());
    glBindTexture(GL_TEXTURE_2D, colorBufferTex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, color1.data());
    glBindTexture(GL_TEXTURE_2D, colorBufferTex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, color2.data());
    glBindTexture(GL_TEXTURE_2D, colorBufferTex[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, color3.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    for (unsigned int i = 0; i < kMaxColorBuffer; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachments[i], GL_TEXTURE_2D,
                               colorBufferTex[i], 0);
    }
    glDrawBuffers(kMaxColorBuffer, &colorAttachments[0]);

    ASSERT_GL_NO_ERROR();

    GLBuffer atomicBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);

    // Reset atomic counter buffer
    GLuint *userCounters;
    userCounters = static_cast<GLuint *>(glMapBufferRange(
        GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    memset(userCounters, 0, sizeof(GLuint));
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    float color[4]      = {1.0f, 0.0f, 0.0f, 1.0f};
    GLint colorLocation = glGetUniformLocation(program, "u_color");
    glUniform4fv(colorLocation, 1, color);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    render(positionLocation, GL_TRUE);

    ASSERT_GL_NO_ERROR();

    // Because no texture is bound, the shader samples black, increments the counter for every pixel
    // and sets all attachments to black.
    for (unsigned int i = 0; i < kMaxColorBuffer; i++)
    {
        glReadBuffer(colorAttachments[i]);
        EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::black);
    }

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer);
    userCounters = static_cast<GLuint *>(
        glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT));
    EXPECT_EQ(*userCounters, kViewportWidth * kViewportHeight * 2);
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Testing that binding the location value using GLES API is conflicted to the location value of the
// fragment inout.
TEST_P(FramebufferFetchES31, FixedUniformLocation)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    constexpr char kVS[] = R"(#version 310 es
in highp vec4 a_position;

void main (void)
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(noncoherent, location = 0) inout highp vec4 o_color;

layout(location = 0) uniform highp vec4 u_color;
void main (void)
{
    o_color += u_color;
})";

    GLProgram program;
    program.makeRaster(kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    std::vector<GLColor> greenColor(kViewportWidth * kViewportHeight, GLColor::green);
    GLTexture colorBufferTex;
    glBindTexture(GL_TEXTURE_2D, colorBufferTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, greenColor.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex, 0);

    ASSERT_GL_NO_ERROR();

    float color[4]      = {1.0f, 0.0f, 0.0f, 1.0f};
    GLint colorLocation = glGetUniformLocation(program, "u_color");
    glUniform4fv(colorLocation, 1, color);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    render(positionLocation, GL_TRUE);

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Verify we can use inout with the default framebuffer
// http://anglebug.com/42265386
TEST_P(FramebufferFetchES31, DefaultFramebufferTest)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 300 es
in highp vec4 a_position;

void main (void)
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) inout highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color += u_color;
})";

    GLProgram program;
    program.makeRaster(kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    // Ensure that we're rendering to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Start with a clear buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    GLint colorLocation    = glGetUniformLocation(program, "u_color");

    // Draw once with red
    glUniform4fv(colorLocation, 1, GLColor::red.toNormalizedVector().data());
    render(positionLocation, GL_FALSE);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Draw again with blue, adding it to the existing red, ending up with magenta
    glUniform4fv(colorLocation, 1, GLColor::blue.toNormalizedVector().data());
    render(positionLocation, GL_FALSE);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
    ASSERT_GL_NO_ERROR();
}

// Verify we can render to the default framebuffer without fetch, then switch to a program
// that does fetch.
// http://anglebug.com/42265386
TEST_P(FramebufferFetchES31, DefaultFramebufferMixedProgramsTest)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 300 es
in highp vec4 a_position;

void main (void)
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
layout(location = 0) out highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color;
})";

    constexpr char kFetchFS[] = R"(#version 300 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) inout highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color += u_color;
})";

    // Create a program that simply writes out a color, no fetching
    GLProgram program;
    program.makeRaster(kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    // Ensure that we're rendering to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Start with a clear buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    GLint colorLocation    = glGetUniformLocation(program, "u_color");

    // Draw once with red
    glUniform4fv(colorLocation, 1, GLColor::red.toNormalizedVector().data());
    render(positionLocation, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Create another program that DOES fetch from the framebuffer
    GLProgram program2;
    program2.makeRaster(kVS, kFetchFS);
    glUseProgram(program2);

    GLint positionLocation2 = glGetAttribLocation(program2, "a_position");
    GLint colorLocation2    = glGetUniformLocation(program2, "u_color");

    // Draw again with blue, fetching red from the framebuffer, adding it together
    glUniform4fv(colorLocation2, 1, GLColor::blue.toNormalizedVector().data());
    render(positionLocation2, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
    ASSERT_GL_NO_ERROR();

    // Switch back to the non-fetched framebuffer, and render green
    glUseProgram(program);
    glUniform4fv(colorLocation, 1, GLColor::green.toNormalizedVector().data());
    render(positionLocation, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Verify we can render to a framebuffer with fetch, then switch to another framebuffer (without
// changing programs) http://anglebug.com/42265386
TEST_P(FramebufferFetchES31, FramebufferMixedFetchTest)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 300 es
in highp vec4 a_position;

void main (void)
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
layout(location = 0) out highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color;
})";

    constexpr char kFetchFS[] = R"(#version 300 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) inout highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color += u_color;
})";

    // Create a program that simply writes out a color, no fetching
    GLProgram program;
    program.makeRaster(kVS, kFS);
    GLint positionLocation = glGetAttribLocation(program, "a_position");
    GLint colorLocation    = glGetUniformLocation(program, "u_color");
    ASSERT_GL_NO_ERROR();

    // Create a program that DOES fetch from the framebuffer
    GLProgram fetchProgram;
    fetchProgram.makeRaster(kVS, kFetchFS);
    GLint fetchPositionLocation = glGetAttribLocation(fetchProgram, "a_position");
    GLint fetchColorLocation    = glGetUniformLocation(fetchProgram, "u_color");
    ASSERT_GL_NO_ERROR();

    // Create an empty framebuffer to use without fetch
    GLFramebuffer framebuffer1;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    std::vector<GLColor> clearColor(kViewportWidth * kViewportHeight, GLColor::transparentBlack);
    GLTexture colorBufferTex1;
    glBindTexture(GL_TEXTURE_2D, colorBufferTex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, clearColor.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex1, 0);
    ASSERT_GL_NO_ERROR();

    // Draw to it with green, without using fetch, overwriting any contents
    glUseProgram(program);
    glUniform4fv(colorLocation, 1, GLColor::green.toNormalizedVector().data());
    render(positionLocation, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::green);
    ASSERT_GL_NO_ERROR();

    // Create another framebuffer to use WITH fetch, and initialize it with blue
    GLFramebuffer framebuffer2;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    std::vector<GLColor> blueColor(kViewportWidth * kViewportHeight, GLColor::blue);
    GLTexture colorBufferTex2;
    glBindTexture(GL_TEXTURE_2D, colorBufferTex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, blueColor.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex2, 0);
    ASSERT_GL_NO_ERROR();

    // Draw once with red, fetching blue from the framebuffer, adding it together
    glUseProgram(fetchProgram);
    glUniform4fv(fetchColorLocation, 1, GLColor::red.toNormalizedVector().data());
    render(fetchPositionLocation, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
    ASSERT_GL_NO_ERROR();

    // Now use the same program (WITH fetch) and render to the other framebuffer that was NOT used
    // with fetch. This verifies the framebuffer state is appropriately updated to match the
    // program.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    render(fetchPositionLocation, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);
    ASSERT_GL_NO_ERROR();
}

// Verify that switching between single sampled framebuffer fetch and multi sampled framebuffer
// fetch works fine
TEST_P(FramebufferFetchES31, SingleSampledMultiSampledMixedTest)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(COHERENT);

    // Create a program that fetches from the framebuffer
    GLProgram fetchProgram;
    fetchProgram.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    GLint positionLocation = glGetAttribLocation(fetchProgram, "a_position");
    GLint colorLocation    = glGetUniformLocation(fetchProgram, "u_color");
    ASSERT_GL_NO_ERROR();

    // Create two single sampled framebuffer
    GLRenderbuffer singleSampledRenderbuffer1;
    glBindRenderbuffer(GL_RENDERBUFFER, singleSampledRenderbuffer1);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    GLFramebuffer singleSampledFramebuffer1;
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              singleSampledRenderbuffer1);

    GLRenderbuffer singleSampledRenderbuffer2;
    glBindRenderbuffer(GL_RENDERBUFFER, singleSampledRenderbuffer2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    GLFramebuffer singleSampledFramebuffer2;
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer2);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              singleSampledRenderbuffer2);

    // Create one multi sampled framebuffer
    GLRenderbuffer multiSampledRenderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, multiSampledRenderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kViewportWidth, kViewportHeight);
    GLFramebuffer multiSampledFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, multiSampledFramebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              multiSampledRenderbuffer);

    // Create a singlesampled render buffer for blit and read
    GLRenderbuffer resolvedRbo;
    glBindRenderbuffer(GL_RENDERBUFFER, resolvedRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    GLFramebuffer resolvedFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, resolvedFbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, resolvedRbo);

    // Clear three Framebuffers with different colors
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::black);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer2);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::blue);

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, multiSampledFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFbo);
    glBlitFramebuffer(0, 0, kViewportWidth, kViewportHeight, 0, 0, kViewportWidth, kViewportHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolvedFbo);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::green);

    // Bind first single sampled framebuffer, draw once with red, fetching black from the
    // framebuffer
    glUseProgram(fetchProgram);
    glUniform4fv(colorLocation, 1, GLColor::red.toNormalizedVector().data());
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer1);
    render(positionLocation, false);
    ASSERT_GL_NO_ERROR();

    // Bind the multi sampled framebuffer, draw once with red, fetching green from the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, multiSampledFramebuffer);
    render(positionLocation, false);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFbo);
    glBlitFramebuffer(0, 0, kViewportWidth, kViewportHeight, 0, 0, kViewportWidth, kViewportHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolvedFbo);
    ASSERT_GL_NO_ERROR();

    // Bind the single sampled framebuffer, draw once with red, fetching blue from the framebuffer
    glUniform4fv(colorLocation, 1, GLColor::red.toNormalizedVector().data());
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer2);
    render(positionLocation, false);
    ASSERT_GL_NO_ERROR();

    // Verify the rendering result on all three framebuffers

    // Verify the last framebuffer being drawn: singleSampledFramebuffer2
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);

    // Verify the second last framebuffer being drawn: multisampledFramebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, multiSampledFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFbo);
    glBlitFramebuffer(0, 0, kViewportWidth, kViewportHeight, 0, 0, kViewportWidth, kViewportHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolvedFbo);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

    // Verify the first framebuffer being drawn: singleSampledFramebuffer1
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer1);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
}

// Verify that calling glFramebufferFetchBarrierEXT without an open render pass is ok.
TEST_P(FramebufferFetchES31, BarrierBeforeDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch") ||
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    glFramebufferFetchBarrierEXT();
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test ARM extension with gl_LastFragColorARM
TEST_P(FramebufferFetchES31, BasicLastFragData_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    setWhichExtension(ARM);

    GLProgram program;
    program.makeRaster(k100VS, getFragmentShader(GLSL100));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    BasicTest(program);
}

// Test ARM extension with multiple draw
TEST_P(FramebufferFetchES31, MultipleDraw_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    setWhichExtension(ARM);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleDrawTest(program);
}

// Testing ARM extension with the order of non-fetch program and fetch program
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetch_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    setWhichExtension(ARM);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchTest(programNonFetch, programFetch);
}

// Testing ARM extension with the order of fetch program and non-fetch program
TEST_P(FramebufferFetchES31, DrawFetchDrawNonFetch_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    setWhichExtension(ARM);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    ASSERT_GL_NO_ERROR();

    DrawFetchDrawNonFetchTest(programNonFetch, programFetch);
}

// Testing ARM extension with framebuffer fetch read in combination with color attachment mask
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetchInStorageBuffer_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    setWhichExtension(ARM);

    GLint maxFragmentShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxFragmentShaderStorageBlocks == 0);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT_WITH_STORAGE_BUFFER));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchInStorageBufferTest(programNonFetch, programFetch,
                                             StorageBufferTestPostFetchAction::Nothing);
}

// Testing ARM extension with framebuffer fetch read in combination with color attachment mask
// and clear
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetchInStorageBufferThenClear_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    setWhichExtension(ARM);

    GLint maxFragmentShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxFragmentShaderStorageBlocks == 0);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT_WITH_STORAGE_BUFFER));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchInStorageBufferTest(programNonFetch, programFetch,
                                             StorageBufferTestPostFetchAction::Clear);
}

// Testing ARM extension with program pipeline
TEST_P(FramebufferFetchES31, ProgramPipeline_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    setWhichExtension(ARM);

    ProgramPipelineTest(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT),
                        getFragmentShader(GLSL310_1ATTACHMENT));
}

// Verify we can use the default framebuffer
// http://anglebug.com/42265386
TEST_P(FramebufferFetchES31, DefaultFramebufferTest_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 300 es
in highp vec4 a_position;

void main (void)
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
#extension GL_ARM_shader_framebuffer_fetch : require
layout(location = 0) out highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color + gl_LastFragColorARM;
})";

    GLProgram program;
    program.makeRaster(kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    // Ensure that we're rendering to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Start with a clear buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    GLint colorLocation    = glGetUniformLocation(program, "u_color");

    // Draw once with red
    glUniform4fv(colorLocation, 1, GLColor::red.toNormalizedVector().data());
    render(positionLocation, GL_FALSE);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Draw again with blue, adding it to the existing red, ending up with magenta
    glUniform4fv(colorLocation, 1, GLColor::blue.toNormalizedVector().data());
    render(positionLocation, GL_FALSE);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
    ASSERT_GL_NO_ERROR();
}

// Verify we can redeclare gl_LastFragColorARM with a new precision
// http://anglebug.com/42265386
TEST_P(FramebufferFetchES31, NondefaultPrecisionTest_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 300 es
in highp vec4 a_position;

void main (void)
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
#extension GL_ARM_shader_framebuffer_fetch : require
highp vec4 gl_LastFragColorARM;
layout(location = 0) out highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color + gl_LastFragColorARM;
})";

    GLProgram program;
    program.makeRaster(kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    // Ensure that we're rendering to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Start with a clear buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    GLint colorLocation    = glGetUniformLocation(program, "u_color");

    // Draw once with red
    glUniform4fv(colorLocation, 1, GLColor::red.toNormalizedVector().data());
    render(positionLocation, GL_FALSE);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Draw again with blue, adding it to the existing red, ending up with magenta
    glUniform4fv(colorLocation, 1, GLColor::blue.toNormalizedVector().data());
    render(positionLocation, GL_FALSE);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
    ASSERT_GL_NO_ERROR();
}

// Verify we can render to the default framebuffer without fetch, then switch to a program
// that does fetch.
// http://anglebug.com/42265386
TEST_P(FramebufferFetchES31, DefaultFramebufferMixedProgramsTest_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 300 es
in highp vec4 a_position;

void main (void)
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
layout(location = 0) out highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color;
})";

    constexpr char kFetchFS[] = R"(#version 300 es
#extension GL_ARM_shader_framebuffer_fetch : require
layout(location = 0) out highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color + gl_LastFragColorARM;
})";

    // Create a program that simply writes out a color, no fetching
    GLProgram program;
    program.makeRaster(kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    // Ensure that we're rendering to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Start with a clear buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    GLint colorLocation    = glGetUniformLocation(program, "u_color");

    // Draw once with red
    glUniform4fv(colorLocation, 1, GLColor::red.toNormalizedVector().data());
    render(positionLocation, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Create another program that DOES fetch from the framebuffer
    GLProgram program2;
    program2.makeRaster(kVS, kFetchFS);
    glUseProgram(program2);

    GLint positionLocation2 = glGetAttribLocation(program2, "a_position");
    GLint colorLocation2    = glGetUniformLocation(program2, "u_color");

    // Draw again with blue, fetching red from the framebuffer, adding it together
    glUniform4fv(colorLocation2, 1, GLColor::blue.toNormalizedVector().data());
    render(positionLocation2, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
    ASSERT_GL_NO_ERROR();

    // Switch back to the non-fetched framebuffer, and render green
    glUseProgram(program);
    glUniform4fv(colorLocation, 1, GLColor::green.toNormalizedVector().data());
    render(positionLocation, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Verify we can render to a framebuffer with fetch, then switch to another framebuffer (without
// changing programs) http://anglebug.com/42265386
TEST_P(FramebufferFetchES31, FramebufferMixedFetchTest_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 300 es
in highp vec4 a_position;

void main (void)
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
layout(location = 0) out highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color;
})";

    constexpr char kFetchFS[] = R"(#version 300 es
#extension GL_ARM_shader_framebuffer_fetch : require
layout(location = 0) out highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color + gl_LastFragColorARM;
})";

    // Create a program that simply writes out a color, no fetching
    GLProgram program;
    program.makeRaster(kVS, kFS);
    GLint positionLocation = glGetAttribLocation(program, "a_position");
    GLint colorLocation    = glGetUniformLocation(program, "u_color");
    ASSERT_GL_NO_ERROR();

    // Create a program that DOES fetch from the framebuffer
    GLProgram fetchProgram;
    fetchProgram.makeRaster(kVS, kFetchFS);
    GLint fetchPositionLocation = glGetAttribLocation(fetchProgram, "a_position");
    GLint fetchColorLocation    = glGetUniformLocation(fetchProgram, "u_color");
    ASSERT_GL_NO_ERROR();

    // Create an empty framebuffer to use without fetch
    GLFramebuffer framebuffer1;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    std::vector<GLColor> clearColor(kViewportWidth * kViewportHeight, GLColor::transparentBlack);
    GLTexture colorBufferTex1;
    glBindTexture(GL_TEXTURE_2D, colorBufferTex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, clearColor.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex1, 0);
    ASSERT_GL_NO_ERROR();

    // Draw to it with green, without using fetch, overwriting any contents
    glUseProgram(program);
    glUniform4fv(colorLocation, 1, GLColor::green.toNormalizedVector().data());
    render(positionLocation, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::green);
    ASSERT_GL_NO_ERROR();

    // Create another framebuffer to use WITH fetch, and initialize it with blue
    GLFramebuffer framebuffer2;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    std::vector<GLColor> blueColor(kViewportWidth * kViewportHeight, GLColor::blue);
    GLTexture colorBufferTex2;
    glBindTexture(GL_TEXTURE_2D, colorBufferTex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, blueColor.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex2, 0);
    ASSERT_GL_NO_ERROR();

    // Draw once with red, fetching blue from the framebuffer, adding it together
    glUseProgram(fetchProgram);
    glUniform4fv(fetchColorLocation, 1, GLColor::red.toNormalizedVector().data());
    render(fetchPositionLocation, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
    ASSERT_GL_NO_ERROR();

    // Now use the same program (WITH fetch) and render to the other framebuffer that was NOT used
    // with fetch. This verifies the framebuffer state is appropriately updated to match the
    // program.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    render(fetchPositionLocation, false);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);
    ASSERT_GL_NO_ERROR();
}

// Verify that switching between single sampled framebuffer fetch and multi sampled framebuffer
// fetch works fine
TEST_P(FramebufferFetchES31, SingleSampledMultiSampledMixedTest_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    setWhichExtension(ARM);

    // Create a program that fetches from the framebuffer
    GLProgram fetchProgram;
    fetchProgram.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    GLint positionLocation = glGetAttribLocation(fetchProgram, "a_position");
    GLint colorLocation    = glGetUniformLocation(fetchProgram, "u_color");
    ASSERT_GL_NO_ERROR();

    // Create two single sampled framebuffer
    GLRenderbuffer singleSampledRenderbuffer1;
    glBindRenderbuffer(GL_RENDERBUFFER, singleSampledRenderbuffer1);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    GLFramebuffer singleSampledFramebuffer1;
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              singleSampledRenderbuffer1);

    GLRenderbuffer singleSampledRenderbuffer2;
    glBindRenderbuffer(GL_RENDERBUFFER, singleSampledRenderbuffer2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    GLFramebuffer singleSampledFramebuffer2;
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer2);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              singleSampledRenderbuffer2);

    // Create one multi sampled framebuffer
    GLRenderbuffer multiSampledRenderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, multiSampledRenderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kViewportWidth, kViewportHeight);
    GLFramebuffer multiSampledFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, multiSampledFramebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              multiSampledRenderbuffer);

    // Create a singlesampled render buffer for blit and read
    GLRenderbuffer resolvedRbo;
    glBindRenderbuffer(GL_RENDERBUFFER, resolvedRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    GLFramebuffer resolvedFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, resolvedFbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, resolvedRbo);

    // Clear three Framebuffers with different colors
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::black);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer2);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::blue);

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, multiSampledFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFbo);
    glBlitFramebuffer(0, 0, kViewportWidth, kViewportHeight, 0, 0, kViewportWidth, kViewportHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolvedFbo);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::green);

    // Bind first single sampled framebuffer, draw once with red, fetching black from the
    // framebuffer
    glUseProgram(fetchProgram);
    glUniform4fv(colorLocation, 1, GLColor::red.toNormalizedVector().data());
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer1);
    render(positionLocation, false);
    ASSERT_GL_NO_ERROR();

    // Bind the multi sampled framebuffer, draw once with red, fetching green from the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, multiSampledFramebuffer);
    render(positionLocation, false);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFbo);
    glBlitFramebuffer(0, 0, kViewportWidth, kViewportHeight, 0, 0, kViewportWidth, kViewportHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolvedFbo);
    ASSERT_GL_NO_ERROR();

    // Bind the single sampled framebuffer, draw once with red, fetching blue from the framebuffer
    glUniform4fv(colorLocation, 1, GLColor::red.toNormalizedVector().data());
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer2);
    render(positionLocation, false);
    ASSERT_GL_NO_ERROR();

    // Verify the rendering result on all three framebuffers

    // Verify the last framebuffer being drawn: singleSampledFramebuffer2
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);

    // Verify the second last framebuffer being drawn: multisampledFramebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, multiSampledFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFbo);
    glBlitFramebuffer(0, 0, kViewportWidth, kViewportHeight, 0, 0, kViewportWidth, kViewportHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolvedFbo);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

    // Verify the first framebuffer being drawn: singleSampledFramebuffer1
    glBindFramebuffer(GL_FRAMEBUFFER, singleSampledFramebuffer1);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
}

// Test ARM extension with new tokens
TEST_P(FramebufferFetchES31, BasicTokenUsage_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));

    // GL_FETCH_PER_SAMPLE_ARM can be set and queried
    GLboolean isFetchPerSampleEnabledBool = false;
    GLint isFetchPerSampleEnabledInt      = -1;
    GLfloat isFetchPerSampleEnabledFloat  = -1.0f;

    // Set GL_FETCH_PER_SAMPLE_ARM true
    glEnable(GL_FETCH_PER_SAMPLE_ARM);
    EXPECT_GL_TRUE(glIsEnabled(GL_FETCH_PER_SAMPLE_ARM));

    // Ensure it returns true
    glGetBooleanv(GL_FETCH_PER_SAMPLE_ARM, &isFetchPerSampleEnabledBool);
    EXPECT_GL_TRUE(isFetchPerSampleEnabledBool);
    glGetIntegerv(GL_FETCH_PER_SAMPLE_ARM, &isFetchPerSampleEnabledInt);
    ASSERT_EQ(isFetchPerSampleEnabledInt, 1);
    glGetFloatv(GL_FETCH_PER_SAMPLE_ARM, &isFetchPerSampleEnabledFloat);
    ASSERT_EQ(isFetchPerSampleEnabledFloat, 1.0);

    // Set GL_FETCH_PER_SAMPLE_ARM false
    glDisable(GL_FETCH_PER_SAMPLE_ARM);
    EXPECT_GL_FALSE(glIsEnabled(GL_FETCH_PER_SAMPLE_ARM));

    // Ensure it returns false
    glGetBooleanv(GL_FETCH_PER_SAMPLE_ARM, &isFetchPerSampleEnabledBool);
    EXPECT_GL_FALSE(isFetchPerSampleEnabledBool);
    glGetIntegerv(GL_FETCH_PER_SAMPLE_ARM, &isFetchPerSampleEnabledInt);
    ASSERT_EQ(isFetchPerSampleEnabledInt, 0);
    glGetFloatv(GL_FETCH_PER_SAMPLE_ARM, &isFetchPerSampleEnabledFloat);
    ASSERT_EQ(isFetchPerSampleEnabledFloat, 0.0);

    ASSERT_GL_NO_ERROR();

    // GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM can only be queried
    GLboolean isFragmentShaderFramebufferFetchMrtBool = false;
    GLint isFragmentShaderFramebufferFetchMrtInt      = -1;
    GLfloat isFragmentShaderFramebufferFetchMrtFloat  = -1.0f;

    // Try to set it, ensure we can't
    glEnable(GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glDisable(GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Ensure we can't query its state with isEnabled
    // Commented out due to http://anglebug.com/42266484
    // glIsEnabled(GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM);
    // EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Ensure GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM returns consistent values
    glGetBooleanv(GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM,
                  &isFragmentShaderFramebufferFetchMrtBool);
    glGetIntegerv(GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM,
                  &isFragmentShaderFramebufferFetchMrtInt);
    ASSERT_EQ(isFragmentShaderFramebufferFetchMrtInt,
              static_cast<GLint>(isFragmentShaderFramebufferFetchMrtBool));
    glGetFloatv(GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM,
                &isFragmentShaderFramebufferFetchMrtFloat);
    ASSERT_EQ(isFragmentShaderFramebufferFetchMrtFloat,
              static_cast<GLfloat>(isFragmentShaderFramebufferFetchMrtBool));

    ASSERT_GL_NO_ERROR();
}

// Test that depth/stencil framebuffer fetch with early_fragment_tests is disallowed
TEST_P(FramebufferFetchES31, NoEarlyFragmentTestsWithDepthStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kDepthFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

layout(early_fragment_tests) in;
highp out vec4 color;

void main()
{
    color = vec4(gl_LastFragDepthARM, 0, 0, 1);
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kDepthFS);
    EXPECT_EQ(0u, shader);

    const char kStencilFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

layout(early_fragment_tests) in;
highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0xE5;
    color = vec4(correct, 0, 0, 1);
})";

    shader = CompileShader(GL_FRAGMENT_SHADER, kStencilFS);
    EXPECT_EQ(0u, shader);
}

// Test using both extensions simultaneously with gl_LastFragData and gl_LastFragColorARM
TEST_P(FramebufferFetchES31, BasicLastFragData_Both)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(BOTH);

    GLProgram program;
    program.makeRaster(k100VS, getFragmentShader(GLSL100));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    BasicTest(program);
}

// Test using both extentions simultaneously with multiple draw
TEST_P(FramebufferFetchES31, MultipleDraw_Both)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(BOTH);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleDrawTest(program);
}

// Testing using both extentions simultaneously with the order of non-fetch program and fetch
// program
TEST_P(FramebufferFetchES31, DrawNonFetchDrawFetch_Both)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(BOTH);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    ASSERT_GL_NO_ERROR();

    DrawNonFetchDrawFetchTest(programNonFetch, programFetch);
}

// Testing using both extentions simultaneously with the order of fetch program and non-fetch
// program
TEST_P(FramebufferFetchES31, DrawFetchDrawNonFetch_Both)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    setWhichExtension(BOTH);

    GLProgram programNonFetch, programFetch;
    programNonFetch.makeRaster(k310VS, getFragmentShader(GLSL310_NO_FETCH_1ATTACHMENT));
    programFetch.makeRaster(k310VS, getFragmentShader(GLSL310_1ATTACHMENT));
    ASSERT_GL_NO_ERROR();

    DrawFetchDrawNonFetchTest(programNonFetch, programFetch);
}

// Testing using both extentions simultaneously with multiple render target, using gl_FragData with
// constant indices
TEST_P(FramebufferFetchES31, MultipleRenderTarget_Both_FragData)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_draw_buffers"));

    GLboolean isFragmentShaderFramebufferFetchMrt = false;
    glGetBooleanv(GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM,
                  &isFragmentShaderFramebufferFetchMrt);
    ANGLE_SKIP_TEST_IF(!isFragmentShaderFramebufferFetchMrt);

    setWhichExtension(BOTH);

    GLProgram program;
    program.makeRaster(k100VS, getFragmentShader(GLSL100_4ATTACHMENT));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL100_4ATTACHMENT);
}

// Testing using both extentions simultaneously with multiple render target, using gl_FragData with
// complex expressions
TEST_P(FramebufferFetchES31, MultipleRenderTarget_Both_FragData_Complex)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_draw_buffers"));

    GLboolean isFragmentShaderFramebufferFetchMrt = false;
    glGetBooleanv(GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM,
                  &isFragmentShaderFramebufferFetchMrt);
    ANGLE_SKIP_TEST_IF(!isFragmentShaderFramebufferFetchMrt);

    setWhichExtension(BOTH);

    GLProgram program;
    program.makeRaster(k100VS, getFragmentShader(GLSL100_COMPLEX));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL100_COMPLEX);
}

// Testing using both extentions simultaneously with multiple render target, using inouts with
// complex expressions
TEST_P(FramebufferFetchES31, MultipleRenderTarget_Both_Complex)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    GLboolean isFragmentShaderFramebufferFetchMrt = false;
    glGetBooleanv(GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM,
                  &isFragmentShaderFramebufferFetchMrt);
    ANGLE_SKIP_TEST_IF(!isFragmentShaderFramebufferFetchMrt);

    setWhichExtension(BOTH);

    GLProgram program;
    program.makeRaster(k310VS, getFragmentShader(GLSL310_COMPLEX));
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    MultipleRenderTargetTest(program, GLSL310_COMPLEX);
}

// Test that using the maximum number of color attachments works.
TEST_P(FramebufferFetchES31, MaximumColorAttachments)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch") &&
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    const WhichExtension whichExtension = chooseBetweenCoherentOrIncoherent();

    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    std::vector<GLTexture> color(maxDrawBuffers);
    std::vector<GLenum> buffers(maxDrawBuffers);
    for (GLint index = 0; index < maxDrawBuffers; ++index)
    {
        buffers[index] = GL_COLOR_ATTACHMENT0 + index;

        glBindTexture(GL_TEXTURE_2D, color[index]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kViewportWidth, kViewportHeight);
        glFramebufferTexture2D(GL_FRAMEBUFFER, buffers[index], GL_TEXTURE_2D, color[index], 0);
    }
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glDrawBuffers(maxDrawBuffers, buffers.data());

    // Create two programs, one to initialize the attachments and another to read back the contents
    // with framebuffer fetch and blend.
    std::ostringstream initFs;
    initFs << "#version 310 es\n";
    for (GLint index = 0; index < maxDrawBuffers; ++index)
    {
        initFs << "layout(location=" << index << ") out highp vec4 color" << index << ";\n";
    }

    std::ostringstream fetchFs;
    fetchFs << makeShaderPreamble(whichExtension, nullptr, maxDrawBuffers);

    initFs << R"(void main()
{
)";
    fetchFs << R"(void main()
{
)";

    for (GLint index = 0; index < maxDrawBuffers; ++index)
    {
        initFs << "  color" << index << " = vec4(" << ((index % 5) / 8.0) << ", "
               << ((index % 4) / 6.0) << ", " << ((index % 3) / 4.0) << ", " << ((index % 2) / 2.0)
               << ");\n";

        fetchFs << "  color" << index << " += vec4(" << (((index + 1) % 2) / 2.0) << ", "
                << (((index + 1) % 3) / 4.0) << ", " << (((index + 1) % 4) / 6.0) << ", "
                << (((index + 1) % 5) / 8.0) << ");\n";
    }

    initFs << "}\n";
    fetchFs << "}\n";

    ANGLE_GL_PROGRAM(init, essl31_shaders::vs::Passthrough(), initFs.str().c_str());
    ANGLE_GL_PROGRAM(fetch, essl31_shaders::vs::Passthrough(), fetchFs.str().c_str());

    drawQuad(init, essl31_shaders::PositionAttrib(), 0.0f);
    if (whichExtension == NON_COHERENT)
    {
        glFramebufferFetchBarrierEXT();
    }
    drawQuad(fetch, essl31_shaders::PositionAttrib(), 0.0f);

    for (GLint index = 0; index < maxDrawBuffers; ++index)
    {
        glReadBuffer(buffers[index]);

        uint32_t expectR = (255 * (index % 5) + 4) / 8;
        uint32_t expectG = (255 * (index % 4) + 3) / 6;
        uint32_t expectB = (255 * (index % 3) + 2) / 4;
        uint32_t expectA = (255 * (index % 2) + 1) / 2;

        expectR += (255 * ((index + 1) % 2) + 1) / 2;
        expectG += (255 * ((index + 1) % 3) + 2) / 4;
        expectB += (255 * ((index + 1) % 4) + 3) / 6;
        expectA += (255 * ((index + 1) % 5) + 4) / 8;

        EXPECT_PIXEL_NEAR(0, 0, expectR, expectG, expectB, expectA, 2);
    }

    ASSERT_GL_NO_ERROR();
}

// Test that depth framebuffer fetch works.
TEST_P(FramebufferFetchES31, Depth)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    color = vec4(gl_LastFragDepthARM, 0, 0, 1);
})";

    GLRenderbuffer color, depth;
    GLFramebuffer fbo;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);

    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kViewportWidth, kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glClearDepthf(0.4f);
    glClear(GL_DEPTH_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(102, 0, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that stencil framebuffer fetch works.
TEST_P(FramebufferFetchES31, Stencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0xE5;
    color = vec4(correct, 0, 0, 1);
})";

    GLRenderbuffer color, stencil;
    GLFramebuffer fbo;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);

    glBindRenderbuffer(GL_RENDERBUFFER, stencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, kViewportWidth, kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glClearStencil(0xE5);
    glClear(GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Test that depth and stencil framebuffer fetch work simultaneously and with the built-ins
// redeclared in the shader.
TEST_P(FramebufferFetchES31, DepthStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

highp float gl_LastFragDepthARM;
highp int gl_LastFragStencilARM;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;
    createFramebufferWithDepthStencil(&color, &depthStencil, &fbo);

    glClearDepthf(0.8f);
    glClearStencil(0x3C);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(255, 204, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that depth and stencil framebuffer fetch works with MSAA.
TEST_P(FramebufferFetchES31, DepthStencilMultisampled)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kViewportWidth, kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);

    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kViewportWidth,
                                     kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glClearDepthf(0.8f);
    glClearStencil(0x3C);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    GLRenderbuffer resolveColor;
    GLFramebuffer resolveFbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFbo);

    glBindRenderbuffer(GL_RENDERBUFFER, resolveColor);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kViewportWidth, kViewportHeight);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              resolveColor);

    glBlitFramebuffer(0, 0, kViewportWidth, kViewportHeight, 0, 0, kViewportWidth, kViewportHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFbo);
    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(255, 204, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that depth and stencil framebuffer fetch works with MSRTT textures.
TEST_P(FramebufferFetchES31, DepthStencilMSRTT)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    GLTexture color, depthStencil;
    GLFramebuffer fbo;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kViewportWidth, kViewportHeight);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color,
                                         0, 4);

    glBindTexture(GL_TEXTURE_2D, depthStencil);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_DEPTH24_STENCIL8, 2 * kViewportWidth, 2 * kViewportHeight);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                                         depthStencil, 1, 4);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glClearDepthf(0.8f);
    glClearStencil(0x3C);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(255, 204, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that depth and stencil framebuffer fetch works with MSRTT renderbuffers.
TEST_P(FramebufferFetchES31, DepthStencilMSRTTRenderbuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 4, GL_RGBA8, kViewportWidth,
                                        kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);

    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kViewportWidth,
                                        kViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glClearDepthf(0.8f);
    glClearStencil(0x3C);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(255, 204, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that depth and stencil framebuffer fetch works with textures
TEST_P(FramebufferFetchES31, DepthStencilTexture)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    GLTexture color, depthStencil;
    GLFramebuffer fbo;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kViewportWidth, kViewportHeight);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

    glBindTexture(GL_TEXTURE_2D, depthStencil);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_DEPTH24_STENCIL8, 2 * kViewportWidth, 2 * kViewportHeight);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencil,
                           1);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glClearDepthf(0.8f);
    glClearStencil(0x3C);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(255, 204, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that depth and stencil framebuffer fetch works with layered framebuffers
TEST_P(FramebufferFetchES31, DepthStencilLayered)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader") &&
                       !IsGLExtensionEnabled("GL_OES_geometry_shader"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    GLTexture color, depthStencil;
    GLFramebuffer fbo;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindTexture(GL_TEXTURE_2D_ARRAY, color);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, kViewportWidth, kViewportHeight, 7);

    glBindTexture(GL_TEXTURE_2D_ARRAY, depthStencil);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH24_STENCIL8, 3 * kViewportWidth,
                 3 * kViewportHeight, 5, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8_OES, nullptr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH24_STENCIL8, 2 * kViewportWidth,
                 2 * kViewportHeight, 7, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8_OES, nullptr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 2, GL_DEPTH24_STENCIL8, kViewportWidth, kViewportHeight, 7, 0,
                 GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8_OES, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 2);

    if (IsGLExtensionEnabled("GL_OES_geometry_shader"))
    {
        glFramebufferTextureOES(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color, 0);
        glFramebufferTextureOES(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthStencil, 2);
    }
    else
    {
        glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color, 0);
        glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthStencil, 2);
    }

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glClearDepthf(0.8f);
    glClearStencil(0x3C);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(255, 204, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that depth and stencil framebuffer fetch works with default framebuffer
TEST_P(FramebufferFetchES31, DepthStencilSurface)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearDepthf(0.8f);
    glClearStencil(0x3C);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(255, 204, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Tests that accessing gl_LastFragDepthARM or gl_LastFragStencilARM without attached depth or
// stencil attachments produces undefined results without generating an error.
TEST_P(FramebufferFetchES31, DrawWithoutDepthAndStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    // Values are undefined
    EXPECT_GL_NO_ERROR();
}

// Similar to DrawWithoutDepthAndStencil, but with a draw call before that does have D/S attachment.
TEST_P(FramebufferFetchES31, DrawWithAndWithoutDepthAndStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    constexpr GLint kWidth  = 37;
    constexpr GLint kHeight = 52;

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);

    glClearColor(0, 0, 0, 0);
    glClearDepthf(0.6);
    glClearStencil(0x3C);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Issue a draw call that correctly renders to half of the framebuffer.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kWidth, kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Remove the depth attachment, and issue another draw call to the other half.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glScissor(0, kHeight / 2, kWidth, kHeight - kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // The bottom half has undefined values, but the top half can be verified.
    EXPECT_PIXEL_NEAR(0, 0, 255, 153, 0, 255, 1);
    EXPECT_PIXEL_NEAR(kWidth - 1, kHeight / 2 - 1, 255, 153, 0, 255, 1);
    EXPECT_GL_NO_ERROR();
}

// Similar to DrawWithoutDepthAndStencil, but with a draw call after that does have D/S attachment.
TEST_P(FramebufferFetchES31, DrawWithoutAndWithDepthAndStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    constexpr GLint kWidth  = 37;
    constexpr GLint kHeight = 52;

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Issue a draw call with undefined render to half of the framebuffer.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kWidth, kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Add a depth attachment, and issue another draw call to the other half.
    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glScissor(0, kHeight / 2, kWidth, kHeight - kHeight / 2);
    glClearDepthf(0.6);
    glClearStencil(0x3C);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // The top half has undefined values, but the bottom half can be verified.
    EXPECT_PIXEL_NEAR(0, kHeight / 2, 255, 153, 0, 255, 1);
    EXPECT_PIXEL_NEAR(kWidth - 1, kHeight - 1, 255, 153, 0, 255, 1);
    EXPECT_GL_NO_ERROR();
}

// Similar to DrawWithAndWithoutDepthAndStencil, but with a framebuffer change instead of attachment
// change.
TEST_P(FramebufferFetchES31, DrawWithAndWithoutDepthAndStencilNewFBO)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    EXPECT_GL_NO_ERROR();

    constexpr GLint kWidth  = 37;
    constexpr GLint kHeight = 52;

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);

    glClearColor(0, 0, 0, 0);
    glClearDepthf(0.6);
    glClearStencil(0x3C);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Issue a draw call that correctly renders to half of the framebuffer.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kWidth, kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Switch framebuffers to the one that doesn't have a depth/stencil attachment.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

    glScissor(0, kHeight / 2, kWidth, kHeight - kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // The bottom half has undefined values, but the top half can be verified.
    EXPECT_PIXEL_NEAR(0, 0, 255, 153, 0, 255, 1);
    EXPECT_PIXEL_NEAR(kWidth - 1, kHeight / 2 - 1, 255, 153, 0, 255, 1);
    EXPECT_GL_NO_ERROR();
}

// Similar to DrawWithoutAndWithDepthAndStencil, but with a framebuffer change instead of attachment
// change.
TEST_P(FramebufferFetchES31, DrawWithoutAndWithDepthAndStencilNewFBO)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    EXPECT_GL_NO_ERROR();

    constexpr GLint kWidth  = 37;
    constexpr GLint kHeight = 52;

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearColor(0, 0, 0, 0);
    glClearDepthf(0.6);
    glClearStencil(0x3C);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);

    // Issue a draw call with undefined render to half of the framebuffer.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kWidth, kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Switch framebuffers to the one that does have a depth/stencil attachment.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

    glScissor(0, kHeight / 2, kWidth, kHeight - kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // The top half has undefined values, but the bottom half can be verified.
    EXPECT_PIXEL_NEAR(0, kHeight / 2, 255, 153, 0, 255, 1);
    EXPECT_PIXEL_NEAR(kWidth - 1, kHeight - 1, 255, 153, 0, 255, 1);
    EXPECT_GL_NO_ERROR();
}

// Similar to DrawWithAndWithoutDepthAndStencil, but framebuffer is MSAA
TEST_P(FramebufferFetchES31, DrawWithAndWithoutDepthAndStencilMSAA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    constexpr GLint kWidth  = 37;
    constexpr GLint kHeight = 52;

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);

    glClearColor(0, 0, 0, 0);
    glClearDepthf(0.6);
    glClearStencil(0x3C);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Issue a draw call that correctly renders to half of the framebuffer.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kWidth, kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Remove the depth attachment, and issue another draw call to the other half.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glScissor(0, kHeight / 2, kWidth, kHeight - kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    GLFramebuffer resolveFbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFbo);
    EXPECT_GL_NO_ERROR();

    GLRenderbuffer resolve;
    glBindRenderbuffer(GL_RENDERBUFFER, resolve);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, resolve);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    // The bottom half has undefined values, but the top half can be verified.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFbo);
    EXPECT_PIXEL_NEAR(0, 0, 255, 153, 0, 255, 1);
    EXPECT_PIXEL_NEAR(kWidth - 1, kHeight / 2 - 1, 255, 153, 0, 255, 1);
    EXPECT_GL_NO_ERROR();
}

// Test that depth and stencil framebuffer fetch work simultaneously and
// verify whether detaching the depth attachment and stencil attachment separately
// works correctly when the renderbuffer internalformat is set to GL_DEPTH24_STENCIL8.
TEST_P(FramebufferFetchES31, DepthStencilDetachSeparatelyD24S8_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    framebufferFetchDepthStencilDetachSeparately(GL_DEPTH24_STENCIL8);

    ASSERT_GL_NO_ERROR();
}

// Test that depth and stencil framebuffer fetch work simultaneously and
// verify whether detaching the depth attachment and stencil attachment separately
// works correctly when the renderbuffer internalformat is set to GL_DEPTH32F_STENCIL8.
TEST_P(FramebufferFetchES31, DepthStencilDetachSeparatelyD32FS8_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    framebufferFetchDepthStencilDetachSeparately(GL_DEPTH32F_STENCIL8);

    ASSERT_GL_NO_ERROR();
}

// Test that framebuffer fetch works as expected when GL_FETCH_PER_SAMPLE_ARM is disabled.
TEST_P(FramebufferFetchES31, DrawFetchPerFragmentAndWriteOut_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    for (auto depthStencilFormat : kDSFormat)
    {
        GLFramebuffer fbo, resolveFbo;
        GLRenderbuffer color[kMaxColorBuffer], depthStencil, resolve;

        bindResolveFboAndVerify(&resolve, &resolveFbo, kViewportWidth, kViewportHeight, false,
                                false, &fbo, depthStencilFormat);

        stateReset();
        createFboWithDepthStencilAndMRT(kViewportWidth, kViewportHeight, 0, depthStencilFormat,
                                        &fbo, color, &depthStencil);
        ASSERT_GL_NO_ERROR();

        ANGLE_GL_PROGRAM(programDS, essl31_shaders::vs::Passthrough(),
                         getFragShaderName(depthStencilFormat));
        ASSERT_GL_NO_ERROR();

        glDisable(GL_FETCH_PER_SAMPLE_ARM);

        clearAndDrawQuad(programDS, false);

        bindResolveFboAndVerify(&resolve, &resolveFbo, kViewportWidth, kViewportHeight, true, false,
                                &fbo, depthStencilFormat);
    }
}

// Test that the discard functionality works as expected during framebuffer fetch when
// GL_FETCH_PER_SAMPLE_ARM is disabled.
TEST_P(FramebufferFetchES31, DrawFetchPerFragmentAndWriteOutWithDiscard_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    const char colorDiscard[] = R"(#version 310 es
precision highp float;

uniform vec4 color;
layout(location=0) out highp vec4 fragColor0;
layout(location=1) out highp vec4 fragColor1;
layout(location=2) out highp vec4 fragColor2;
layout(location=3) out highp vec4 fragColor3;

void main()
{
    ivec2 fragCoord = ivec2(gl_FragCoord.xy);

    if (0 == ((fragCoord.x + fragCoord.y) % 2)) discard;

    fragColor0 = fragColor1 = fragColor2 = fragColor3 = color;
})";

    GLFramebuffer fbo, resolveFbo;
    GLRenderbuffer color[kMaxColorBuffer], depthStencil, resolve;

    bindResolveFboAndVerify(&resolve, &resolveFbo, 2, 2, false, true, &fbo, GL_DEPTH_COMPONENT24);

    stateReset();
    createFboWithDepthStencilAndMRT(2, 2, 0, GL_DEPTH_COMPONENT24, &fbo, color, &depthStencil);
    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(programColorDiscard, essl31_shaders::vs::Passthrough(), colorDiscard);
    ASSERT_GL_NO_ERROR();

    glDisable(GL_FETCH_PER_SAMPLE_ARM);

    clearAndDrawQuad(programColorDiscard, true);

    bindResolveFboAndVerify(&resolve, &resolveFbo, 2, 2, true, true, &fbo, GL_DEPTH_COMPONENT24);
}

// Test that framebuffer fetch works as expected under the conditions of multisample and with
// GL_FETCH_PER_SAMPLE_ARM disabled.
TEST_P(FramebufferFetchES31, DrawFetchPerFragmentAndWriteOutWithMultisample_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    for (auto depthStencilFormat : kDSFormat)
    {
        for (int samples = 2; samples <= maxSamplesSupported(depthStencilFormat); samples *= 2)
        {
            if (!sampleCountSupported(GL_RENDERBUFFER, depthStencilFormat, samples))
            {
                continue;
            }

            GLFramebuffer fbo, resolveFbo;
            GLRenderbuffer color[kMaxColorBuffer], depthStencil, resolve;

            bindResolveFboAndVerify(&resolve, &resolveFbo, kViewportWidth, kViewportHeight, false,
                                    false, &fbo, depthStencilFormat);

            stateReset();
            createFboWithDepthStencilAndMRT(kViewportWidth, kViewportHeight, samples,
                                            depthStencilFormat, &fbo, color, &depthStencil);
            ASSERT_GL_NO_ERROR();

            ANGLE_GL_PROGRAM(programDS, essl31_shaders::vs::Passthrough(),
                             getFragShaderName(depthStencilFormat));
            ASSERT_GL_NO_ERROR();

            glDisable(GL_FETCH_PER_SAMPLE_ARM);

            clearAndDrawQuad(programDS, false);

            bindResolveFboAndVerify(&resolve, &resolveFbo, kViewportWidth, kViewportHeight, true,
                                    false, &fbo, depthStencilFormat);
        }
    }
}

// Test that framebuffer fetch works as expected under the conditions of multisample and with
// GL_FETCH_PER_SAMPLE_ARM enabled.
TEST_P(FramebufferFetchES31, DrawFetchPerSampleAndWriteOutWithMultisample_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    for (auto depthStencilFormat : kDSFormat)
    {
        for (int samples = 2; samples <= maxSamplesSupported(depthStencilFormat); samples *= 2)
        {
            if (!sampleCountSupported(GL_RENDERBUFFER, depthStencilFormat, samples))
            {
                continue;
            }

            GLFramebuffer fbo, resolveFbo;
            GLRenderbuffer color[kMaxColorBuffer], depthStencil, resolve;

            bindResolveFboAndVerify(&resolve, &resolveFbo, kViewportWidth, kViewportHeight, false,
                                    false, &fbo, depthStencilFormat);

            stateReset();
            createFboWithDepthStencilAndMRT(kViewportWidth, kViewportHeight, samples,
                                            depthStencilFormat, &fbo, color, &depthStencil);
            ASSERT_GL_NO_ERROR();

            ANGLE_GL_PROGRAM(programDS, essl31_shaders::vs::Passthrough(),
                             getFragShaderName(depthStencilFormat));
            ASSERT_GL_NO_ERROR();

            glEnable(GL_FETCH_PER_SAMPLE_ARM);

            clearAndDrawQuad(programDS, false);

            bindResolveFboAndVerify(&resolve, &resolveFbo, kViewportWidth, kViewportHeight, true,
                                    false, &fbo, depthStencilFormat);
        }
    }
}

// Test that the discard functionality works as expected during framebuffer fetch with multisample
// when GL_FETCH_PER_SAMPLE_ARM is disabled.
TEST_P(FramebufferFetchES31, DrawFetchPerFragmentAndWriteOutWithDiscardAndMultisample_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    const char colorDiscard[] = R"(#version 310 es
precision highp float;

uniform vec4 color;
layout(location=0) out highp vec4 fragColor0;
layout(location=1) out highp vec4 fragColor1;
layout(location=2) out highp vec4 fragColor2;
layout(location=3) out highp vec4 fragColor3;

void main()
{
    ivec2 fragCoord = ivec2(gl_FragCoord.xy);

    if (0 == ((fragCoord.x + fragCoord.y) % 2)) discard;

    fragColor0 = fragColor1 = fragColor2 = fragColor3 = color;
})";

    for (int samples = 2; samples <= maxSamplesSupported(GL_DEPTH_COMPONENT16); samples *= 2)
    {
        if (!sampleCountSupported(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, samples))
        {
            continue;
        }

        GLFramebuffer fbo, resolveFbo;
        GLRenderbuffer color[kMaxColorBuffer], depthStencil, resolve;

        bindResolveFboAndVerify(&resolve, &resolveFbo, 2, 2, false, true, &fbo,
                                GL_DEPTH_COMPONENT24);

        stateReset();
        createFboWithDepthStencilAndMRT(2, 2, samples, GL_DEPTH_COMPONENT24, &fbo, color,
                                        &depthStencil);
        ASSERT_GL_NO_ERROR();

        ANGLE_GL_PROGRAM(programColorDiscard, essl31_shaders::vs::Passthrough(), colorDiscard);
        ASSERT_GL_NO_ERROR();

        glDisable(GL_FETCH_PER_SAMPLE_ARM);

        clearAndDrawQuad(programColorDiscard, true);

        bindResolveFboAndVerify(&resolve, &resolveFbo, 2, 2, true, true, &fbo,
                                GL_DEPTH_COMPONENT24);
    }
}

// Test that the discard functionality works as expected during framebuffer fetch with multisample
// when GL_FETCH_PER_SAMPLE_ARM is enabled.
TEST_P(FramebufferFetchES31, DrawFetchPerSampleAndWriteOutWithDiscardAndMultisample_ARM)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    const char colorDiscard[] = R"(#version 310 es
precision highp float;

uniform vec4 color;
layout(location=0) out highp vec4 fragColor0;
layout(location=1) out highp vec4 fragColor1;
layout(location=2) out highp vec4 fragColor2;
layout(location=3) out highp vec4 fragColor3;

void main()
{
    ivec2 fragCoord = ivec2(gl_FragCoord.xy);

    if (0 == ((fragCoord.x + fragCoord.y) % 2)) discard;

    fragColor0 = fragColor1 = fragColor2 = fragColor3 = color;
})";

    for (int samples = 2; samples <= maxSamplesSupported(GL_DEPTH_COMPONENT16); samples *= 2)
    {
        if (!sampleCountSupported(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, samples))
        {
            continue;
        }

        GLFramebuffer fbo, resolveFbo;
        GLRenderbuffer color[kMaxColorBuffer], depthStencil, resolve;

        bindResolveFboAndVerify(&resolve, &resolveFbo, 2, 2, false, true, &fbo,
                                GL_DEPTH_COMPONENT24);

        stateReset();
        createFboWithDepthStencilAndMRT(2, 2, samples, GL_DEPTH_COMPONENT24, &fbo, color,
                                        &depthStencil);
        ASSERT_GL_NO_ERROR();

        ANGLE_GL_PROGRAM(programColorDiscard, essl31_shaders::vs::Passthrough(), colorDiscard);
        ASSERT_GL_NO_ERROR();

        glEnable(GL_FETCH_PER_SAMPLE_ARM);

        clearAndDrawQuad(programColorDiscard, true);

        bindResolveFboAndVerify(&resolve, &resolveFbo, 2, 2, true, true, &fbo,
                                GL_DEPTH_COMPONENT24);
    }
}

// Similar to DrawWithoutAndWithDepthAndStencil, but framebuffer is MSAA
TEST_P(FramebufferFetchES31, DrawWithoutAndWithDepthAndStencilMSAA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    constexpr GLint kWidth  = 37;
    constexpr GLint kHeight = 52;

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Issue a draw call with undefined render to half of the framebuffer.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kWidth, kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Add a depth attachment, and issue another draw call to the other half.
    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glScissor(0, kHeight / 2, kWidth, kHeight - kHeight / 2);
    glClearDepthf(0.6);
    glClearStencil(0x3C);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    GLFramebuffer resolveFbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFbo);
    EXPECT_GL_NO_ERROR();

    GLRenderbuffer resolve;
    glBindRenderbuffer(GL_RENDERBUFFER, resolve);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, resolve);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    // The top half has undefined values, but the bottom half can be verified.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFbo);
    EXPECT_PIXEL_NEAR(0, kHeight / 2, 255, 153, 0, 255, 1);
    EXPECT_PIXEL_NEAR(kWidth - 1, kHeight - 1, 255, 153, 0, 255, 1);
    EXPECT_GL_NO_ERROR();
}

// Similar to DrawWithAndWithoutDepthAndStencilMSAA, but color framebuffer fetch is simultaneously
// used.
TEST_P(FramebufferFetchES31, DrawWithAndWithoutDepthAndStencilAndColorMSAA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp inout vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color.xy = vec2(correct, gl_LastFragDepthARM);
    color.zw += vec2(0.25, 0.5);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    constexpr GLint kWidth  = 37;
    constexpr GLint kHeight = 52;

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);

    glClearColor(0, 0, 0, 0);
    glClearDepthf(0.6);
    glClearStencil(0x3C);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Issue a draw call that correctly renders to half of the framebuffer.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kWidth, kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Remove the depth attachment, and issue another draw call to the other half.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glScissor(0, kHeight / 2, kWidth, kHeight - kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    GLFramebuffer resolveFbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFbo);
    EXPECT_GL_NO_ERROR();

    GLRenderbuffer resolve;
    glBindRenderbuffer(GL_RENDERBUFFER, resolve);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, resolve);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    // The bottom half has undefined values, but the top half can be verified.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFbo);
    EXPECT_PIXEL_NEAR(0, 0, 255, 153, 63, 127, 1);
    EXPECT_PIXEL_NEAR(kWidth - 1, kHeight / 2 - 1, 255, 153, 63, 127, 1);
    EXPECT_GL_NO_ERROR();
}

// Similar to DrawWithoutAndWithDepthAndStencilMSAA, but color framebuffer fetch is simultaneously
// used.
TEST_P(FramebufferFetchES31, DrawWithoutAndWithDepthAndStencilAndColorMSAA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp inout vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color.xy = vec2(correct, gl_LastFragDepthARM);
    color.zw += vec2(0.25, 0.5);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    constexpr GLint kWidth  = 37;
    constexpr GLint kHeight = 52;

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Issue a draw call with undefined render to half of the framebuffer.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kWidth, kHeight / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Add a depth attachment, and issue another draw call to the other half.
    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glScissor(0, kHeight / 2, kWidth, kHeight - kHeight / 2);
    glClearDepthf(0.6);
    glClearStencil(0x3C);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    GLFramebuffer resolveFbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFbo);
    EXPECT_GL_NO_ERROR();

    GLRenderbuffer resolve;
    glBindRenderbuffer(GL_RENDERBUFFER, resolve);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, resolve);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    // The top half has undefined values, but the bottom half can be verified.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFbo);
    EXPECT_PIXEL_NEAR(0, kHeight / 2, 255, 153, 63, 127, 1);
    EXPECT_PIXEL_NEAR(kWidth - 1, kHeight - 1, 255, 153, 63, 127, 1);
    EXPECT_GL_NO_ERROR();
}

// Test that depth and stencil framebuffer fetch works with pbuffers
TEST_P(FramebufferFetchES31, DepthStencilPbuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x3C;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    EGLWindow *window = getEGLWindow();
    ASSERT(window);
    EGLConfig config   = window->getConfig();
    EGLContext context = window->getContext();
    EGLDisplay dpy     = window->getDisplay();
    EGLint surfaceType = 0;

    // Skip if pbuffer surface is not supported
    eglGetConfigAttrib(dpy, config, EGL_SURFACE_TYPE, &surfaceType);
    ANGLE_SKIP_TEST_IF((surfaceType & EGL_PBUFFER_BIT) == 0);

    const EGLint surfaceWidth        = static_cast<EGLint>(getWindowWidth());
    const EGLint surfaceHeight       = static_cast<EGLint>(getWindowHeight());
    const EGLint pBufferAttributes[] = {
        EGL_WIDTH, surfaceWidth, EGL_HEIGHT, surfaceHeight, EGL_NONE,
    };

    // Create Pbuffer surface
    EGLSurface pbufferSurface = eglCreatePbufferSurface(dpy, config, pBufferAttributes);
    ASSERT_NE(pbufferSurface, EGL_NO_SURFACE);
    ASSERT_EGL_SUCCESS();

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, pbufferSurface, pbufferSurface, context));
    ASSERT_EGL_SUCCESS();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearDepthf(0.8f);
    glClearStencil(0x3C);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(255, 204, 0, 255));
    ASSERT_GL_NO_ERROR();

    // Switch back to the window surface and destroy the pbuffer
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, window->getSurface(), window->getSurface(), context));
    ASSERT_EGL_SUCCESS();

    EXPECT_EGL_TRUE(eglDestroySurface(dpy, pbufferSurface));
    ASSERT_EGL_SUCCESS();
}

// Test that depth framebuffer fetch works with color framebuffer fetch
TEST_P(FramebufferFetchES31, DepthAndColor)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch") &&
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    const WhichExtension whichExtension = chooseBetweenCoherentOrIncoherent();

    std::ostringstream fs;
    fs << makeShaderPreamble(
        whichExtension, "#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require", 1);
    fs << R"(void main()
{
    color0 = vec4(gl_LastFragDepthARM, 0, 0, 1);
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;
    createFramebufferWithDepthStencil(&color, &depthStencil, &fbo);

    glClearDepthf(0.4f);
    glClear(GL_DEPTH_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), fs.str().c_str());
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(102, 0, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that stencil framebuffer fetch works with color framebuffer fetch
TEST_P(FramebufferFetchES31, StencilAndColor)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch") &&
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    const WhichExtension whichExtension = chooseBetweenCoherentOrIncoherent();

    std::ostringstream fs;
    fs << makeShaderPreamble(
        whichExtension, "#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require", 1);
    fs << R"(void main()
{
    bool correct = gl_LastFragStencilARM == 0x7D;
    color0 = vec4(correct, 0, 0, 1);
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;
    createFramebufferWithDepthStencil(&color, &depthStencil, &fbo);

    glClearStencil(0x7D);
    glClear(GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), fs.str().c_str());
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(255, 0, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that depth/stencil framebuffer fetch works with color framebuffer fetch
TEST_P(FramebufferFetchES31, DepthStencilAndColor)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch") &&
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    const WhichExtension whichExtension = chooseBetweenCoherentOrIncoherent();

    std::ostringstream fs;
    fs << makeShaderPreamble(
        whichExtension, "#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require", 1);
    fs << R"(void main()
{
    bool correct = gl_LastFragStencilARM == 0x7D;
    color0 = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;
    createFramebufferWithDepthStencil(&color, &depthStencil, &fbo);

    glClearDepthf(0.8f);
    glClearStencil(0x7D);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), fs.str().c_str());
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(255, 204, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that mixing depth-only and stencil-only framebuffer fetch programs work
TEST_P(FramebufferFetchES31, DepthThenStencilThenNone)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kDepthFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    color = vec4(gl_LastFragDepthARM, 0, 0, 1);
})";

    const char kStencilFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0xE5;
    color = vec4(0, correct, 0, 1);
})";

    const char kNoneFS[] = R"(#version 310 es

highp out vec4 color;

void main()
{
    color = vec4(0, 0, 1, 1);
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;
    createFramebufferWithDepthStencil(&color, &depthStencil, &fbo);

    glClearDepthf(0.8f);
    glClearStencil(0xE5);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(depth, essl31_shaders::vs::Passthrough(), kDepthFS);
    ANGLE_GL_PROGRAM(stencil, essl31_shaders::vs::Passthrough(), kStencilFS);
    ANGLE_GL_PROGRAM(none, essl31_shaders::vs::Passthrough(), kNoneFS);

    drawQuad(depth, essl31_shaders::PositionAttrib(), 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    drawQuad(stencil, essl31_shaders::PositionAttrib(), 0.0f);
    drawQuad(none, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(204, 255, 255, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that starting without framebuffer fetch, then doing framebuffer fetch works.
TEST_P(FramebufferFetchES31, NoneThenDepthThenStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kDepthFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    color = vec4(gl_LastFragDepthARM, 0, 0, 1);
})";

    const char kStencilFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0xE5;
    color = vec4(0, correct, 0, 1);
})";

    const char kNoneFS[] = R"(#version 310 es

highp out vec4 color;

void main()
{
    color = vec4(0, 0, 1, 1);
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;
    createFramebufferWithDepthStencil(&color, &depthStencil, &fbo);

    glClearDepthf(0.4f);
    glClearStencil(0xE5);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(depth, essl31_shaders::vs::Passthrough(), kDepthFS);
    ANGLE_GL_PROGRAM(stencil, essl31_shaders::vs::Passthrough(), kStencilFS);
    ANGLE_GL_PROGRAM(none, essl31_shaders::vs::Passthrough(), kNoneFS);

    drawQuad(none, essl31_shaders::PositionAttrib(), 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    drawQuad(depth, essl31_shaders::PositionAttrib(), 0.0f);
    drawQuad(stencil, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(102, 255, 255, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that depth/stencil framebuffer fetch is actually coherent by writing to depth/stencil in one
// draw call and reading from it in another.
TEST_P(FramebufferFetchES31, DepthStencilDrawThenRead)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kWriteDepthFS[] = R"(#version 310 es

highp out vec4 color;

void main()
{
    if (gl_FragCoord.x < 8.)
        gl_FragDepth = 0.4f;
    else
        gl_FragDepth = 0.8f;
    color = vec4(0, 0, 1, 1);
})";

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x5B;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;
    createFramebufferWithDepthStencil(&color, &depthStencil, &fbo);

    glClearDepthf(0);
    glClearStencil(0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(writeDepth, essl31_shaders::vs::Passthrough(), kWriteDepthFS);
    ANGLE_GL_PROGRAM(read, essl31_shaders::vs::Passthrough(), kFS);

    // Write depth (0.4 or 0.8 by the shader) and stencil (0x5B) in one draw call
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_TRUE);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x5B, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    drawQuad(writeDepth, essl31_shaders::PositionAttrib(), 0.0f);

    // Read them in the next draw call to verify
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    drawQuad(read, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth / 2, kViewportHeight, GLColor(255, 102, 255, 255));
    EXPECT_PIXEL_RECT_EQ(kViewportWidth / 2, 0, kViewportWidth - kViewportWidth, kViewportHeight,
                         GLColor(255, 204, 255, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that writing to gl_FragDepth does not affect gl_LastFragDepthARM.
TEST_P(FramebufferFetchES31, DepthWriteAndReadInSameShader)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    gl_FragDepth = 0.9;
    color = vec4(gl_LastFragDepthARM, 0, 0, 1);
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;
    createFramebufferWithDepthStencil(&color, &depthStencil, &fbo);

    glClearDepthf(0.4f);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_TRUE);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(102, 0, 0, 255));
    ASSERT_GL_NO_ERROR();

    // For completeness, verify that gl_FragDepth did write to depth.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);

    ANGLE_GL_PROGRAM(red, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(green, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    drawQuad(red, essl1_shaders::PositionAttrib(), 0.79f);
    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor::red);

    drawQuad(green, essl1_shaders::PositionAttrib(), 0.81f);
    EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test that render pass can start with D/S framebuffer fetch, then color framebuffer fetch is used.
TEST_P(FramebufferFetchES31, DepthStencilThenColor)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch") &&
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    const WhichExtension whichExtension = chooseBetweenCoherentOrIncoherent();

    const char kDepthStencilFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x7D;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    std::ostringstream colorFS;
    colorFS << makeShaderPreamble(whichExtension, nullptr, 1);
    colorFS << R"(void main()
{
    color0.x /= 2.;
    color0.y *= 2.;
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;
    createFramebufferWithDepthStencil(&color, &depthStencil, &fbo);

    glClearDepthf(0.4f);
    glClearStencil(0x7D);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ANGLE_GL_PROGRAM(readDepthStencil, essl31_shaders::vs::Passthrough(), kDepthStencilFS);
    ANGLE_GL_PROGRAM(readColor, essl31_shaders::vs::Passthrough(), colorFS.str().c_str());

    drawQuad(readDepthStencil, essl31_shaders::PositionAttrib(), 0.0f);
    drawQuad(readColor, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(127, 204, 0, 255), 1);
    EXPECT_PIXEL_COLOR_NEAR(kViewportWidth - 1, kViewportHeight - 1, GLColor(127, 204, 0, 255), 1);
    ASSERT_GL_NO_ERROR();
}

// Test that render pass can start without framebuffer fetch, then do D/S framebuffer fetch, then
// color framebuffer fetch.  This test uses PPOs.
TEST_P(FramebufferFetchES31, NoneThenDepthStencilThenColorPPO)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch") &&
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    const WhichExtension whichExtension = chooseBetweenCoherentOrIncoherent();

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kNoneFS[] = R"(#version 310 es

highp out vec4 color;

void main()
{
    color = vec4(0, 0, 1, 1);
})";

    const char kDepthStencilFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require

highp out vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x7D;
    color = vec4(correct, gl_LastFragDepthARM, 0, 1);
})";

    std::ostringstream colorFS;
    colorFS << makeShaderPreamble(whichExtension, nullptr, 1);
    colorFS << R"(void main()
{
    color0.x /= 2.;
    color0.y *= 2.;
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;
    createFramebufferWithDepthStencil(&color, &depthStencil, &fbo);

    GLProgramPipeline nonePPO, depthStencilPPO, colorPPO;
    makeProgramPipeline(nonePPO, kVS, kNoneFS);
    makeProgramPipeline(depthStencilPPO, kVS, kDepthStencilFS);
    makeProgramPipeline(colorPPO, kVS, colorFS.str().c_str());

    glClearDepthf(0.4f);
    glClearStencil(0x7D);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBindProgramPipeline(nonePPO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glBindProgramPipeline(depthStencilPPO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisable(GL_BLEND);

    glBindProgramPipeline(colorPPO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(127, 204, 255, 255), 1);
    EXPECT_PIXEL_COLOR_NEAR(kViewportWidth - 1, kViewportHeight - 1, GLColor(127, 204, 255, 255),
                            1);
    ASSERT_GL_NO_ERROR();
}

// Test that using the maximum number of color attachments works in conjunction with depth/stencil
// framebuffer fetch.
TEST_P(FramebufferFetchES31, MaximumColorAttachmentsAndDepthStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch") &&
                       !IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));
    const WhichExtension whichExtension = chooseBetweenCoherentOrIncoherent();

    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture depthStencil;
    std::vector<GLTexture> color(maxDrawBuffers);
    std::vector<GLenum> buffers(maxDrawBuffers);
    for (GLint index = 0; index < maxDrawBuffers; ++index)
    {
        buffers[index] = GL_COLOR_ATTACHMENT0 + index;

        glBindTexture(GL_TEXTURE_2D, color[index]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kViewportWidth, kViewportHeight);
        glFramebufferTexture2D(GL_FRAMEBUFFER, buffers[index], GL_TEXTURE_2D, color[index], 0);
    }

    glBindTexture(GL_TEXTURE_2D, depthStencil);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, kViewportWidth, kViewportHeight);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencil,
                           0);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glDrawBuffers(maxDrawBuffers, buffers.data());

    glClearColor(0, 0, 1, 0);
    glClearDepthf(0.8f);
    glClearStencil(0x7D);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    std::ostringstream fs;
    fs << makeShaderPreamble(whichExtension,
                             "#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require",
                             maxDrawBuffers);
    fs << R"(void main()
{
    bool correct = gl_LastFragStencilARM == 0x7D;
)";
    for (GLint index = 0; index < maxDrawBuffers; ++index)
    {
        fs << "  color" << index << " += vec4(correct, gl_LastFragDepthARM, 0, 1);\n";
    }
    fs << "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), fs.str().c_str());

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    for (GLint index = 0; index < maxDrawBuffers; ++index)
    {
        glReadBuffer(buffers[index]);
        EXPECT_PIXEL_RECT_EQ(0, 0, kViewportWidth, kViewportHeight, GLColor(255, 204, 255, 255));
    }
    ASSERT_GL_NO_ERROR();
}

// Test that depth/stencil framebuffer fetch works with advanced blend
TEST_P(FramebufferFetchAndAdvancedBlendES31, DepthStencilAndAdvancedBlend)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_shader_framebuffer_fetch_depth_stencil"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_KHR_blend_equation_advanced"));

    const char kFS[] = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : require
#extension GL_KHR_blend_equation_advanced : require

layout(blend_support_multiply) out;
layout(location = 0) out mediump vec4 color;

void main()
{
    bool correct = gl_LastFragStencilARM == 0x7D;
    color = vec4(correct, gl_LastFragDepthARM, 0, 0.5);
})";

    GLRenderbuffer color, depthStencil;
    GLFramebuffer fbo;
    createFramebufferWithDepthStencil(&color, &depthStencil, &fbo);

    glClearColor(0.5, 0.2, 0.4, 0.6);
    glClearDepthf(0.8f);
    glClearStencil(0x7D);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendEquation(GL_MULTIPLY_KHR);

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);

    EXPECT_PIXEL_NEAR(0, 0, 255, 148, 51, 204, 1);
    EXPECT_PIXEL_NEAR(kViewportWidth - 1, kViewportHeight - 1, 255, 148, 51, 204, 1);
    ASSERT_GL_NO_ERROR();
}

// Test switching between framebuffer fetch and non framebuffer fetch draw calls, with multiple
// calls in each mode in between.  Tests Vulkan backend's emulation of coherent framebuffer fetch
// over non-coherent hardware.  While this is untestable without adding counters, the test should
// generate implicit framebuffer fetch barriers only when the current program uses framebuffer
// fetch.  This can be observed in RenderDoc.
TEST_P(FramebufferFetchES31, SwitchWithAndWithoutFramebufferFetchPrograms)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    // Program without framebuffer fetch
    constexpr char kFS1[] = R"(#version 310 es
layout(location = 0) out highp vec4 o_color;
uniform mediump vec4 u_color;
void main (void)
{
    o_color = u_color;
})";
    ANGLE_GL_PROGRAM(drawColor, kVS, kFS1);
    glUseProgram(drawColor);
    GLint uniLoc = glGetUniformLocation(drawColor, "u_color");
    ASSERT_NE(uniLoc, -1);

    // Program with framebuffer fetch
    constexpr char kFS2[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) inout highp vec4 o_color;
void main (void)
{
    o_color = o_color * o_color + vec4(0.1, 0.2, 0.3, 0.2);
})";
    ANGLE_GL_PROGRAM(ff, kVS, kFS2);

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kViewportWidth, kViewportHeight);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glClearColor(0, 0, 0.5, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Start without framebuffer fetch.
    glUseProgram(drawColor);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glUniform4f(uniLoc, 0.7, 0, 0, 0.3);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glUniform4f(uniLoc, 0.1, 0.4, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Switch to framebuffer fetch mode, and draw a few times
    glDisable(GL_BLEND);
    glUseProgram(ff);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Break the render pass.  Later continue drawing in framebuffer fetch mode without changing
    // programs to ensure that framebuffer fetch barrier is still added.
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(165, 84, 153, 72), 3);

    // More FF calls
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Back to no FF calls, no barrier should be added.
    glEnable(GL_BLEND);
    glUseProgram(drawColor);
    glUniform4f(uniLoc, 0.2, 0.1, 0.05, 0.15);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Verify results
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(145, 100, 201, 109), 3);
}

// Test that declaring inout variables but only ever writing to them works.
TEST_P(FramebufferFetchES31, WriteOnlyInOutVariable)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 310 es
in highp vec4 position;
void main (void)
{
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require

layout(location = 0) inout highp vec4 color;

void main (void)
{
    color = vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glClearColor(0, 1, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, "position", 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
}

// Test that declaring inout variables but only ever writing to them works.  This test writes to
// different channels of the variable separately.
TEST_P(FramebufferFetchES31, WriteOnlyInOutVariableSplit)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 310 es
in highp vec4 position;
void main (void)
{
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require

layout(location = 0) inout highp vec4 color;

void main (void)
{
    color.xz = vec2(1, 0);
    color.wy = vec2(1, 1);
    return;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glClearColor(0, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, "position", 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::yellow);
}

// Verify that partial writes to an |inout| variable don't make ANGLE consider it as an |out|
// variable.
TEST_P(FramebufferFetchES31, WriteOnlyInOutVariablePartial)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 310 es
in highp vec4 position;
void main (void)
{
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require

layout(location = 0) inout highp vec4 color;

void main (void)
{
    color.x = 1.;
    color.wy = vec2(1, 0);
    return;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glClearColor(0, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, "position", 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::magenta);
}

// Verify that conditional writes to an |inout| variable don't make ANGLE consider it as an |out|
// variable.
TEST_P(FramebufferFetchES31, WriteOnlyInOutVariableConditional)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 310 es
in highp vec4 position;
void main (void)
{
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require

layout(location = 0) inout highp vec4 color;

void main (void)
{
    if (gl_FragCoord.x < 8.)
    {
        color.yz = vec2(1, 0);
    }
    color.x = 1.;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, "position", 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, getWindowHeight(), GLColor::yellow);
    EXPECT_PIXEL_RECT_EQ(8, 0, getWindowWidth() - 8, getWindowHeight(), GLColor::magenta);
}

// Verify that early out from main stops write-only |inout| variables from turning into |out|
// variables.
TEST_P(FramebufferFetchES31, WriteOnlyInOutVariableEarlyReturn)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 310 es
in highp vec4 position;
void main (void)
{
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require

layout(location = 0) inout highp vec4 color;

void main (void)
{
    if (gl_FragCoord.x < 8.)
    {
        return;
    }
    color.x = 1.;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, "position", 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, getWindowHeight(), GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(8, 0, getWindowWidth() - 8, getWindowHeight(), GLColor::magenta);
}

// Verify that discard in the shader stops write-only |inout| variables from turning into |out|
// variables.
TEST_P(FramebufferFetchES31, WriteOnlyInOutVariableDiscard)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    constexpr char kVS[] = R"(#version 310 es
in highp vec4 position;
void main (void)
{
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require

layout(location = 0) inout highp vec4 color;

void f()
{
    if (gl_FragCoord.x < 8.)
    {
        discard;
    }
}

void main (void)
{
    f();
    color.x = 1.;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, "position", 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, getWindowHeight(), GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(8, 0, getWindowWidth() - 8, getWindowHeight(), GLColor::magenta);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(FramebufferFetchES31);
ANGLE_INSTANTIATE_TEST_ES31_AND(FramebufferFetchES31,
                                ES31_VULKAN().disable(Feature::SupportsSPIRV14));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(FramebufferFetchAndAdvancedBlendES31);
ANGLE_INSTANTIATE_TEST_ES31_AND(FramebufferFetchAndAdvancedBlendES31,
                                ES31_VULKAN_SWIFTSHADER()
                                    .disable(Feature::SupportsBlendOperationAdvanced)
                                    .enable(Feature::EmulateAdvancedBlendEquations));
}  // namespace angle
