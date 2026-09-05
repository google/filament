// RUN: not %dxc -T cs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

[[vk::binding(0)]] RWByteAddressBuffer src;
#define LOCAL_SIZE 32

[[vk::constant_id(0)]] int gBufferSize;
[[vk::constant_id(1)]] int gSetValue;

[numthreads(LOCAL_SIZE, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
  if ((4 * dispatchThreadID.x) < gBufferSize)
    src.Store4(4 * dispatchThreadID.x, uint4(gSetValue, gSetValue, gSetValue, gSetValue));
}

// CHECK: :6:28: error: missing default value for specialization constant
