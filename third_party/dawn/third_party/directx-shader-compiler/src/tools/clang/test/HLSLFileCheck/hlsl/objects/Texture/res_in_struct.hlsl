// RUN: %dxc -T lib_6_3 -default-linkage external %s | FileCheck %s

// resources in return/params disallowed for lib_6_3
// CHECK: error: Exported function
// CHECK: emit
// CHECK: must not contain a resource in parameter or return type.

struct M {
   float3 a;
   Texture2D<float4> tex;
};

float4 emit(M m)  {
   return m.tex.Load(m.a);
}