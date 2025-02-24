//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

layout(push_constant) uniform PushConstants {
    // Translation from source to destination coordinates.
    CoordType offset;
    vec2 stretch;
    vec2 invSrcExtent;
    int srcLayer;
    int samples;
    float invSamples;
    // Mask to output only to color attachments that are actually present.
    int outputMask;
    // Flip control.
    bool flipX;
    bool flipY;
    bool rotateXY;
} params;

CoordType getSrcImageCoords()
{
    // Assume only one direction; x.  We are blitting from source to destination either flipped or
    // not, with a stretch factor of T.  If resolving, T == 1.  Note that T here is:
    //
    //     T = SrcWidth / DstWidth
    //
    // Assume the blit offset in source is S and in destination D.  If flipping, S has the
    // coordinates of the opposite size of the rectangle.  In this shader, we have the fragment
    // coordinates, X, which is a point in the destination buffer.  We need to map this to the
    // source buffer to know where to sample from.
    //
    // If there's no flipping:
    //
    //     S Y             D    X
    //     +-x----+   ->   +----x-----------+
    //
    //        Y = S + (X - D) * T
    //     => Y = TX - (DT - S)
    //
    // If there's flipping:
    //
    //          Y S        D    X
    //     +----x-+   ->   +----x-----------+
    //
    //        Y = S - (X - D) * T
    //     => Y = -(TX - (DT + S))
    //
    // The above can be implemented as:
    //
    //     !Flip: Y = TX - O      where O = DT-S
    //      Flip: Y = -(TX - O)   where O = DT+S
    //
    // Note that T is params.stretch and O is params.offset.

    CoordType srcImageCoords = CoordType(gl_FragCoord.xy);  // X
#if !IsResolve
    srcImageCoords *= params.stretch;                       // TX
#endif
    srcImageCoords -= params.offset;                        // TX - O

    // If flipping, negate the coordinates.
    if (params.flipX)
        srcImageCoords.x = -srcImageCoords.x;
    if (params.flipY)
        srcImageCoords.y = -srcImageCoords.y;
    if (params.rotateXY)
        srcImageCoords.xy = srcImageCoords.yx;

    return srcImageCoords;
}

#if IsBlitColor
void broadcastColor(ColorType colorValue)
{
    // Note: not exporting to render targets that are not present optimizes the number of export
    // instructions, which would have otherwise been a likely bottleneck.
    if ((params.outputMask & (1 << 0)) != 0)
    {
        colorOut0 = colorValue;
    }
    if ((params.outputMask & (1 << 1)) != 0)
    {
        colorOut1 = colorValue;
    }
    if ((params.outputMask & (1 << 2)) != 0)
    {
        colorOut2 = colorValue;
    }
    if ((params.outputMask & (1 << 3)) != 0)
    {
        colorOut3 = colorValue;
    }
    if ((params.outputMask & (1 << 4)) != 0)
    {
        colorOut4 = colorValue;
    }
    if ((params.outputMask & (1 << 5)) != 0)
    {
        colorOut5 = colorValue;
    }
    if ((params.outputMask & (1 << 6)) != 0)
    {
        colorOut6 = colorValue;
    }
    if ((params.outputMask & (1 << 7)) != 0)
    {
        colorOut7 = colorValue;
    }
}
#endif
