// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s  | FileCheck %s

// CHECK: %"class.Texture2D<vector<float, 4> >" = type { <4 x float>
// CHECK: %"class.Texture2D<float>" = type { float

typedef snorm float snorm_float;

Texture2D<float4> T1a;
Texture2D<snorm_float> T2a;
float foo2() { return T1a.Load(1).x * T2a.Load(2); }
