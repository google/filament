// RUN: %dxc -T lib_6_8 -fspv-target-env=vulkan1.3 -Od -spirv %s | FileCheck %s

// CHECK-DAG:                                          OpCapability RuntimeDescriptorArray
// CHECK-DAG:                                          OpExtension "SPV_EXT_descriptor_indexing"

// CHECK-DAG:            [[uint_00:%[_a-zA-Z0-9]+]] = OpConstantComposite %v2uint %uint_0 %uint_0
// CHECK-DAG:          [[float_0000:%[_a-zA-Z0-9]+]] = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// CHECK-DAG:             [[image_t:%[_a-zA-Z0-9]+]] = OpTypeImage %float 2D 2 0 0 2 Rgba32f
// CHECK-DAG:       [[ptr_u_image_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[image_t]]
// CHECK-DAG:          [[ra_image_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[image_t]]
// CHECK-DAG:    [[ptr_u_ra_image_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_image_t]]

// CHECK-DAG: OpDecorate %ResourceDescriptorHeap DescriptorSet 0
// CHECK-DAG: OpDecorate %ResourceDescriptorHeap Binding 0

// CHECK: %ResourceDescriptorHeap = OpVariable [[ptr_u_ra_image_t]] UniformConstant

// CHECK: [[ptr:%[_a-zA-Z0-9]+]] = OpAccessChain [[ptr_u_image_t]] %ResourceDescriptorHeap %uint_0
// CHECK: [[img:%[_a-zA-Z0-9]+]] = OpLoad %type_2d_image [[ptr]]
// CHECK: OpImageWrite [[img]] [[uint_00]] [[float_0000]] None

static RWTexture2D<float4> OutputTexture = ResourceDescriptorHeap[0];

[shader("raygeneration")]
void RayGenMain() {
  OutputTexture[uint2(0, 0)] = float4(0, 0, 0, 0);
}
