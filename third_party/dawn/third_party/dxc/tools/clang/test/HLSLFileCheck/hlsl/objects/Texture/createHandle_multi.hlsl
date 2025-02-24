// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s  | FileCheck %s

// CHECK: %"class.Texture2D<float>" = type { float
// CHECK: %"class.Texture2D<vector<float, 4> >" = type { <4 x float>

Texture2D<float> T1;
Texture2D<float4> T2;
float foo() { return T1.Load(1) * T2.Load(2); }
