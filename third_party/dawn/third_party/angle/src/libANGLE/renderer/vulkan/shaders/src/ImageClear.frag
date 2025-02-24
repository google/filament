//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageClear.frag: Clear image to a solid color.

#version 450 core

#if IsFloat
#define Type vec4
#elif IsSint
#define Type ivec4
#elif IsUint
#define Type uvec4
#else
#error "Not all formats are accounted for"
#endif

#if Attachment0
#define ATTACHMENT 0
#elif Attachment1
#define ATTACHMENT 1
#elif Attachment2
#define ATTACHMENT 2
#elif Attachment3
#define ATTACHMENT 3
#elif Attachment4
#define ATTACHMENT 4
#elif Attachment5
#define ATTACHMENT 5
#elif Attachment6
#define ATTACHMENT 6
#elif Attachment7
#define ATTACHMENT 7
#else
#error "Not all attachment index possibilities are accounted for"
#endif

layout(push_constant) uniform PushConstants {
    Type clearColor;
    float clearDepth;
} params;

layout(location = ATTACHMENT) out Type colorOut;

void main()
{
    colorOut = params.clearColor;
#if ClearDepth
    gl_FragDepth = params.clearDepth;
#endif
}
