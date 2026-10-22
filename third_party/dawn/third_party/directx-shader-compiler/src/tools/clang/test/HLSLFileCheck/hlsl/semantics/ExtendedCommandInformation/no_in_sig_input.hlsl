// RUN: %dxc -E main -T vs_6_8 %s | FileCheck %s

// CHECK: Extended command info
// CHECK: @main

// CHECK: call i32 @dx.op.startInstanceLocation.i32(i32 257)
// CHECK: call i32 @dx.op.startVertexLocation.i32(i32 256)

// Make sure no input element is generated for the entry point.
// CHECK: !{void ()* @main, !"main", ![[SIG:[0-9]+]], null, ![[extAttr:[0-9]+]]}
// The input should be null
// CHECK: ![[SIG]] = !{null,

// tag 0: ShaderFlags, 274877906944 = SampleCmpGradientOrBias
// CHECK: ![[extAttr]] = !{i32 0, i64 274877906944}

float4 main(int loc : SV_StartVertexLocation
           , uint loc2 : SV_StartInstanceLocation
           ) : SV_Position
{
    float4 r = 0;
    r += loc;
    r += loc2;
    return r;
}
