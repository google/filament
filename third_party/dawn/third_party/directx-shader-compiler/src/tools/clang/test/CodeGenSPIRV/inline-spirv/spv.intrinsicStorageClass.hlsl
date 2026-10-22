// RUN: %dxc -T ps_6_0 -E main -spirv -Vd -fcgl  %s -spirv | FileCheck %s

[[vk::ext_extension("SPV_KHR_ray_tracing")]]
[[vk::ext_capability(/* RayTracingKHR */ 4479)]]
[[vk::ext_storage_class(/*RayPayloadKHR*/5338)]]
float4 payload;
// CHECK-DAG: [[ptr_payload_v4:%[a-zA-Z0-9_]+]] = OpTypePointer RayPayloadKHR %v4float
// CHECK-DAG: %payload = OpVariable [[ptr_payload_v4]] RayPayloadKHR

int main() : SV_Target0 {

  [[vk::ext_storage_class(/* CrossWorkgroup */ 5)]]
  int foo = 3;
// CHECK-DAG:     [[ptr_cw_int:%[a-zA-Z0-9_]+]] = OpTypePointer CrossWorkgroup %int
// CHECK-DAG:     %foo = OpVariable [[ptr_cw_int]] CrossWorkgroup

  return foo;
}
