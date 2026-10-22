// RUN: %dxc -E main -T vs_6_6 -enable-16bit-types  %s | FileCheck %s
// CHECK: %dx.types.ResourceProperties { i32 4364, i32 16 })  ; AnnotateHandle(res,props)  resource: RWStructuredBuffer<stride=16>
// CHECK: %dx.types.ResourceProperties { i32 268, i32 16 })  ; AnnotateHandle(res,props)  resource: StructuredBuffer<stride=16>

struct S1 {
   float16_t4 v1;
};

struct S {
  uint16_t4 v0;
  S1 v2;
};

StructuredBuffer<S> srv;
RWStructuredBuffer<S> uav;

void main(uint i : IN0)
{
    uav[i] = srv[i];
}
