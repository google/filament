// Test that the deprecated option, select-validator, doesn't work.
// RUN: not %dxc -E main -T vs_6_7 -select-validator internal %s 2>&1 | FileCheck %s

// CHECK: dxc failed : Unknown argument: '-select-validator'

float4 main(int loc : SV_StartVertexLocation
           , uint loc2 : SV_StartInstanceLocation
           ) : SV_Position
{
    float4 r = 0;
    r += loc;
    r += loc2;
    return r;
}
