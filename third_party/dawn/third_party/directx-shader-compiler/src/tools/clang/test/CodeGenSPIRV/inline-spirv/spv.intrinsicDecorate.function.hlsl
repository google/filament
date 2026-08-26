// RUN: %dxc -T cs_6_0 -E main -fcgl %s -spirv | FileCheck %s

// Inline-SPIR-V attributes placed directly on a function must be honored:
// vk::ext_decorate emits an OpDecorate targeting the OpFunction, and
// vk::ext_capability / vk::ext_extension add the capability / extension to the
// module. Known SPIR-V enums are used here so the module still validates; the
// mechanism is identical for vendor-specific decoration numbers.

// CHECK-DAG: OpCapability Int8
// CHECK-DAG: OpExtension "some_extension"

[[vk::ext_capability(/* Int8 */ 39)]]
[[vk::ext_extension("some_extension")]]
[[vk::ext_decorate(/* RelaxedPrecision */ 0)]]
[noinline] uint Identity(uint x) { return x; }

// Decorate a second function to test the hashing in the module's decoration set.
[[vk::ext_decorate(/* RelaxedPrecision */ 0)]]
[noinline] uint Increment(uint x) { return x + 1; }

// CHECK-DAG: OpDecorate %Identity RelaxedPrecision
// CHECK-DAG: OpDecorate %Increment RelaxedPrecision

// CHECK-DAG: %Identity = OpFunction %uint DontInline
// CHECK-DAG: %Increment = OpFunction %uint DontInline

RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {
  buf[0] = Identity(tid.x) + Increment(tid.x);
}
