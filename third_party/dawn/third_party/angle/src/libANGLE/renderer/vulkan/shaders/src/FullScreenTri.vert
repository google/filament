//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FullScreenTri.vert: Simple full-screen triangle vertex shader.

#version 450 core

const vec2 kVertices[] = {
    vec2(-1, -1),
    vec2(3, -1),
    vec2(-1, 3),
};

void main()
{
    gl_Position = vec4(kVertices[gl_VertexIndex], 0, 1);
}
