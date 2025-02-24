// RUN: %dxc -E main -T vs_6_6 -enable-16bit-types  %s | FileCheck %s
// CHECK: %dx.types.ResourceProperties { i32 4876, i32 32 })  ; AnnotateHandle(res,props)  resource: RWStructuredBuffer<stride=32>
// CHECK: %dx.types.ResourceProperties { i32 780, i32 32 })  ; AnnotateHandle(res,props)  resource: StructuredBuffer<stride=32>

struct S1 {
   float16_t4 v1;
   float v2; 
   double v3;
};

struct S {
  uint16_t4 v0;
  S1 v4;
};

StructuredBuffer<S> srv;
RWStructuredBuffer<S> uav;

void main(uint i : IN0)
{
    uav[i] = srv[i];
}
