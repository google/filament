// RUN: %dxc -auto-binding-space 13 -default-linkage external -T lib_6_3 %s | %D3DReflect %s | FileCheck %s

float DoubleMAD(float a, float b, float c) {
  return (float)((double)a * (double)b + (double)c);
}

[shader("vertex")]
float4 VSMain(float4 In : IN) : SV_Position {
  return In * DoubleMAD(In.x, In.y, In.z);
}

[shader("pixel")]
[earlydepthstencil]
float4 PSMain(float4 In : IN, out float Depth : SV_Depth) : SV_Target {
  Depth = In.z;
  return In;
}

// CHECK: ID3D12LibraryReflection:
// CHECK:     FunctionCount: 3
// CHECK-LABEL:     D3D12_FUNCTION_DESC: Name: \01?DoubleMAD{{[@$?.A-Za-z0-9_]+}}
// CHECK:       RequiredFeatureFlags: 0x1
// CHECK-LABEL:     D3D12_FUNCTION_DESC: Name: PSMain
// CHECK:       RequiredFeatureFlags: 0x2
// CHECK-LABEL:     D3D12_FUNCTION_DESC: Name: VSMain
// CHECK:       RequiredFeatureFlags: 0x1
