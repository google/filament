// RUN: %dxc -T cs_6_5 -E main -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

struct MeshPart {
  uint indexOffset;
  uint positionOffset;
  uint normalOffset;
  uint texCoord0Offset;
};

// CHECK: %_ptr_StorageBuffer_type_StructuredBuffer_MeshPart = OpTypePointer StorageBuffer %type_StructuredBuffer_MeshPart
// CHECK: %_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint

StructuredBuffer<MeshPart> g_meshParts[] : register(t2, space1);
RWStructuredBuffer<uint> g_output : register(u1, space2);

// CHECK: %g_meshParts = OpVariable %_ptr_StorageBuffer__runtimearr_type_StructuredBuffer_MeshPart StorageBuffer
// CHECK: %g_output = OpVariable %_ptr_StorageBuffer_type_RWStructuredBuffer_uint StorageBuffer

[numthreads(64, 1, 1)]
void main()
{
  MeshPart meshPart = g_meshParts[0][0];
  g_output[0] = meshPart.indexOffset;
// CHECK: OpAccessChain %_ptr_StorageBuffer_type_StructuredBuffer_MeshPart %g_meshParts %int_0
// CHECK: OpAccessChain %_ptr_StorageBuffer_uint %g_output %int_0 %uint_0
}
