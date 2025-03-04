// RUN: not %dxc -T cs_6_0 -E main %s -spirv -fspv-max-id 30 2>&1 | FileCheck %s --check-prefix=CHECK-30
// RUN: %dxc -T cs_6_0 -E main %s -spirv -fspv-max-id 400 2>&1 | FileCheck %s --check-prefix=CHECK-400

// With a lower limit, there will be an ID overflow in the optimizer. Note that
// the error message can vary depending on where the optimizer fails. This test
// is low enough to fail as early as possible leading to a consistent error
// message.
// CHECK-30: fatal error: failed to optimize SPIR-V: ID overflow. Try running compact-ids.

// With a larger limit, the test case can compile successfully.
// CHECK-400: Bound: 204


RWStructuredBuffer<int> data;

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{
  [[unroll]]
  for( int i = 0; i < 64; i++ ) {
    data[i] = i;
  }
}