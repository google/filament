// RUN: %dxc -T ps_6_0 -E main -spirv -Vd -fcgl  %s -spirv | FileCheck %s

//CHECK: [[payloadTy:%[a-zA-Z0-9_]+]] = OpTypeStruct %v4float
//CHECK-NEXT: [[payloadTyPtr:%[a-zA-Z0-9_]+]] = OpTypePointer RayPayloadKHR [[payloadTy]]
//CHECK: [[crossTy:%[a-zA-Z0-9_]+]] = OpTypePointer CrossWorkgroup %int
//CHECK: {{%[a-zA-Z0-9_]+}} = OpVariable [[payloadTyPtr]] RayPayloadKHR
//CHECK: {{%[a-zA-Z0-9_]+}} = OpVariable [[crossTy]] CrossWorkgroup

[[vk::ext_storage_class(/*RayPayloadKHR*/5338)]]
float4 payload;

int main() : SV_Target0 {
  [[vk::ext_storage_class(/* CrossWorkgroup */ 5)]] int foo = 3;
  return foo;
}
