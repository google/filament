// RUN: %dxc -T cs_6_0 -E main -fcgl -Vd %s -spirv | FileCheck %s
RWBuffer<int> buffer1 : register(u1);
RWByteAddressBuffer buffer2 : register(u2);

// CHECK: [[Resource:%[a-zA-Z0-9_]+]] = OpTypeStruct %type_RWByteAddressBuffer %uint
struct Resource {
  RWByteAddressBuffer rwbuffer;
  uint offset;
};

[numthreads(8, 1, 1)]
void main(uint globalId : SV_DispatchThreadID,
          uint localId  : SV_GroupThreadID,
          uint groupId  : SV_GroupID) {
// CHECK: [[buffer2:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Uniform_type_RWByteAddressBuffer Uniform
  Resource resourceInfo2 = {buffer2, 2};
// CHECK: OpCompositeConstruct [[Resource]] [[buffer2]] %uint_2
  buffer1[0] = resourceInfo2.rwbuffer.Load(0);
}
