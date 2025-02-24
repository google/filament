//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ExportStencil.frag: Helper to emulate functionality of VK_EXT_shader_stencil_export where
// missing.  This shader reads a single stencil bit and discards or not based on that.  The stencil
// mask is used to turn that into a bit set on the stencil buffer.

#version 450 core

layout(input_attachment_index = 0, set = 0, binding = 0) uniform usubpassInput stencilIn;

layout(push_constant) uniform PushConstants {
    uint bit;
} params;

void main()
{
    uint stencilValue = subpassLoad(stencilIn).x;
    if ((stencilValue >> params.bit & 1u) == 0)
    {
        discard;
    }
}
