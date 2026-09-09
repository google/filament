// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %SI0 InputAttachmentIndex 0
// CHECK: OpDecorate %SI1 InputAttachmentIndex 1
// CHECK: OpDecorate %SI2 InputAttachmentIndex 2

// CHECK-NOT: OpDecorate %SI3 InputAttachmentIndex

// CHECK: OpDecorate %SI1 DescriptorSet 0
// CHECK: OpDecorate %SI1 Binding 5

// CHECK: OpDecorate %SI2 DescriptorSet 3
// CHECK: OpDecorate %SI2 Binding 5

// CHECK: OpDecorate %SI0 DescriptorSet 0
// CHECK: OpDecorate %SI0 Binding 0

// CHECK: OpDecorate %SI3 DescriptorSet 0
// CHECK: OpDecorate %SI3 Binding 1

[[vk::input_attachment_index(0)]]
SubpassInput SI0;

[[vk::input_attachment_index(1), vk::binding(5)]]
SubpassInput SI1;

[[vk::input_attachment_index(2), vk::binding(5, 3)]]
SubpassInput SI2;

SubpassInput SI3;

void main() {

}
