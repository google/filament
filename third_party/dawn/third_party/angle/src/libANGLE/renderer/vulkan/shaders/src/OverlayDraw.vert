//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OverlayDraw.vert: Set up overlay widgets for drawing.  A maximum of 32 text widgets and 32 graph
// widgets is supported simultaneously.  One vkCmdDraw call is issued for all text widgets, and one
// for all graph widgets; with the number of instances depicting the number of text or graph
// widgets.  The shader produces four vertices of a triangle strip, covering the area affected by
// the widget.

#version 450 core

#extension GL_GOOGLE_include_directive : require

#include "OverlayDraw.inc"

layout(location = 0) flat out uint widgetIndex;

void main()
{
    widgetIndex = gl_InstanceIndex;
    const uvec4 widgetCoords = params.isText
        ? textWidgetsData[widgetIndex].coordinates
        : graphWidgetsData[widgetIndex].coordinates;

    // Generate the following quad:
    //
    // Vertex 2                 Vertex 3
    //     +-----------------------+
    //     |                       |
    //     |                       |
    //     +-----------------------+
    // Vertex 0                 Vertex 1
    //
    // Vertex0: widgetCoords.xy
    // Vertex1: widgetCoords.zy
    // Vertex2: widgetCoords.xw
    // Vertex3: widgetCoords.zw

    bool isLeft = (gl_VertexIndex & 1) == 0;
    bool isBottom = (gl_VertexIndex & 2) == 0;

    uvec2 position = uvec2(isLeft ? widgetCoords.x : widgetCoords.z,
                           isBottom ? widgetCoords.y : widgetCoords.w);

    if (params.rotateXY)
    {
        // Rotate by 90 degrees
        position = position.yx;
        position.x = params.viewportSize.x - position.x;
    }

    gl_Position = vec4((vec2(position) / vec2(params.viewportSize)) * 2. - 1., 0, 1);
}
