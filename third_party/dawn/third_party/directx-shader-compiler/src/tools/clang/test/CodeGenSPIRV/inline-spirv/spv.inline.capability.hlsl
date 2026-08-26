// RUN: %dxc -T ps_6_0 -E main -fcgl -Vd %s -spirv | FileCheck %s

// CHECK-DAG: OpCapability Int8
// CHECK-DAG: OpCapability SampleMaskPostDepthCoverage
// CHECK-DAG: OpCapability WorkgroupMemoryExplicitLayoutKHR


// Test that the capability on a typedef is added to the module.
[[vk::ext_capability(/* Int8 */ 39)]]
typedef vk::SpirvType</* OpTypeInt */ 21, 8, 8, vk::Literal<vk::integral_constant<uint, 8> >, vk::Literal<vk::integral_constant<bool, false> > > uint8_t;

// Test that the capability on a variable is added to the module.
[[vk::ext_capability(/* SampleMaskPostDepthCoverageCapability */ 4447)]]
uint8_t val;

// Test that the capability on the entry point is added to the module.
[[vk::ext_capability(/* WorkgroupMemoryExplicitLayoutKHR */ 4428)]]
void main() {
}
