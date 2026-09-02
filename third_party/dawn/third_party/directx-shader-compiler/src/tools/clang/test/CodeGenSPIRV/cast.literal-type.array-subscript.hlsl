// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

RWStructuredBuffer<uint> Out;
groupshared uint Mem[1];
[numthreads(1, 1, 1)]
void main() {
  // CHECK: [[sub:%[0-9]+]] = OpISub %int %int_1 %int_1
  // CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_Workgroup_uint %Mem [[sub]]
  Mem[1 - 1] = 0;
  Out[0] = Mem[0];
}

