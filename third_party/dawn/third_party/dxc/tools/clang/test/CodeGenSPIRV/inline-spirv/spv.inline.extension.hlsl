// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK-DAG: OpExtension "entry_point_extension"
// CHECK-DAG: OpExtension "another_extension"
// CHECK-DAG: OpExtension "some_extension"
// CHECK-DAG: OpExtension "ext_on_field1"
// CHECK-DAG: OpExtension "ext_on_field2"

[[vk::ext_extension("some_extension"), vk::ext_extension("another_extension")]]
int val;

struct T
{
  [[vk::ext_extension("ext_on_field1"), vk::ext_extension("ext_on_field2")]]
  int val;
};

[[vk::ext_extension("entry_point_extension")]]
void main() {
  T t;
  int local = val+t.val;
}
