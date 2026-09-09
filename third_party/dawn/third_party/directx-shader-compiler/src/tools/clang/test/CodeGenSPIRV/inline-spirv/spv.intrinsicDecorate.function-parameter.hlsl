// RUN: %dxc -T ps_6_0 -E main -fcgl -Vd -spirv -fcgl %s -spirv | FileCheck %s

void SimpleAdd([[vk::ext_decorate(/* spv::DecorationAliased */ 20)]] inout float a,
               [[vk::ext_decorate(/* spv::DecorationAliased */ 20)]] inout float b) {
  a += 1.0;
  b += 2.0;
}

// CHECK: OpDecorate %x Aliased
// CHECK: OpDecorate %a Aliased
// CHECK: OpDecorate %b Aliased
// CHECK: %SimpleAdd = OpFunction %void None {{%[a-zA-Z0-9_]+}}
// CHECK-NEXT: %a = OpFunctionParameter %_ptr_Function_float
// CHECK-NEXT: %b = OpFunctionParameter %_ptr_Function_float

float4 main() : SV_Target {
  [[vk::ext_decorate(/*spv::DecorationAliased*/20)]] float x = 43.0;
  SimpleAdd(x, x);
  return float4(x, x, x, x);
}