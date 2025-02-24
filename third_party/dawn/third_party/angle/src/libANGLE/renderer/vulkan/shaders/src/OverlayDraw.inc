//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Limits:
// Note that max length and size is defined such that each buffer length is below 16KB, the minimum
// guaranteed supported size for uniform buffers.
#define MAX_TEXT_WIDGETS 32
#define MAX_GRAPH_WIDGETS 32

#define MAX_TEXT_LENGTH 256
#define MAX_GRAPH_SIZE 256

// Font information:
#define FONT_CHARACTERS 95

#extension GL_EXT_samplerless_texture_functions : require

struct TextWidgetData
{
    uvec4 coordinates;
    vec4 color;
    uvec4 fontSize;  // w unused.  xy has the font glyph width/height.  z has the mip.
    uvec4 text[MAX_TEXT_LENGTH / 16];
};

struct GraphWidgetData
{
    uvec4 coordinates;
    vec4 color;
    uvec4 valueWidth;  // yzw unused.  x should necessarily divide coordinate's z-x
    uvec4 values[MAX_GRAPH_SIZE / 4];
};

layout(set = 0, binding = 0) uniform TextWidgets
{
    TextWidgetData textWidgetsData[MAX_TEXT_WIDGETS];
};

layout(set = 0, binding = 1) uniform GraphWidgets
{
    GraphWidgetData graphWidgetsData[MAX_GRAPH_WIDGETS];
};

layout(set = 0, binding = 2) uniform texture2DArray font;

layout(push_constant) uniform PushConstants
{
    uvec2 viewportSize;
    // Whether drawing text or graph widgets
    bool isText;
    // Prerotation support
    bool rotateXY;
} params;
