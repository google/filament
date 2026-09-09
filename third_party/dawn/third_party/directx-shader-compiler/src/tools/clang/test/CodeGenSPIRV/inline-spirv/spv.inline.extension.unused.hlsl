// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK-NOT: OpExtension "ext_on_type"
// CHECK-NOT: OpCapability Int8
// CHECK-NOT: OpExtension "ext_on_field"
// CHECK-NOT: OpCapability StorageBuffer8BitAccess

// Test that the capabilities and extensions on an unused type are not added
// to the Spir-V module.
using uint8_t [[vk::ext_capability(/* Int8 */ 39), vk::ext_extension("ext_on_type")]] = vk::SpirvType</* OpTypeInt */ 21, 8, 8, vk::Literal<vk::integral_constant<uint, 8> >, vk::Literal<vk::integral_constant<bool, false> > >;

// Test that the capabilities and extensions on a field in an unused struct are
// not added to the Spir-V module.
struct T {
  [[vk::ext_extension("ext_on_field")]]
  [[vk::ext_capability(/* StorageBuffer8BitAccess */ 4448)]]
  int v;
};

void main() {
}
