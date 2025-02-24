// RUN: %dxc -E main -T vs_6_6 -enable-16bit-types  %s | FileCheck %s
// CHECK: %dx.types.ResourceProperties { i32 4620, i32 28 })  ; AnnotateHandle(res,props)  resource: RWStructuredBuffer<stride=28>
// CHECK: %dx.types.ResourceProperties { i32 524, i32 28 })  ; AnnotateHandle(res,props)  resource: StructuredBuffer<stride=28>

struct S1 {
   float16_t4 v1;
   float v2;
};

struct S {
  uint4 v0;
  S1 v3;
};

StructuredBuffer<S> srv;
RWStructuredBuffer<S> uav;

void main(uint i : IN0)
{
    uav[i] = srv[i];
}
