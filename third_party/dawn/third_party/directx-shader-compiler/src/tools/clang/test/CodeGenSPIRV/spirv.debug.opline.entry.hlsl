// RUN: %dxc -T ps_6_0 -E main -Zi -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opline.entry.hlsl

float4 main(float2 a : TEXCOORD0,
            float3 b : NORMAL,
            float4 c : COLOR) : SV_Target {
// CHECK:                  OpLine [[file]] 6 1
// CHECK-NEXT: %main = OpFunction %void None
// CHECK:                  OpLine [[file]] 6 1
// CHECK-NEXT: %src_main = OpFunction %v4float None
// CHECK-NEXT:             OpLine [[file]] 6 20
// CHECK-NEXT:        %a = OpFunctionParameter %_ptr_Function_v2float
// CHECK-NEXT:             OpLine [[file]] 7 20
// CHECK-NEXT:        %b = OpFunctionParameter %_ptr_Function_v3float
// CHECK-NEXT:             OpLine [[file]] 8 20
// CHECK-NEXT:        %c = OpFunctionParameter %_ptr_Function_v4float
  float4 d = float4(a, b.xy + c.zw);
  return d;
}
