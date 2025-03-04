// RUN: %dxc -E main -T cs_6_2 -fcgl  %s -spirv | FileCheck %s


struct StructA
{
  float3 one;
  float3 two;
};

uint foo()
{
  return sizeof(StructA);
}

[numthreads(64, 1, 1)]
void main()
{
  uint a = foo();
// CHECK: OpStore %b %uint_24
  uint b = sizeof(StructA);
}
// CHECK: %foo = OpFunction
// CHECK: OpReturnValue %uint_24
