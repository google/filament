// RUN: %dxc -E main -T cs_6_0 %s  | FileCheck %s

// Expect non-uniform resource index (i1 true):
// CHECK: call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 {{.*}}, i1 true)

struct CBType
{
  float cbvals[4];
};

ConstantBuffer<CBType> CBs[100] : register(b0);
RWBuffer<uint> Output;

[numthreads(4,1,1)]
void main(uint index : SV_GroupThreadID)
{
  Output[index] = CBs[NonUniformResourceIndex(index)].cbvals[0];
}
