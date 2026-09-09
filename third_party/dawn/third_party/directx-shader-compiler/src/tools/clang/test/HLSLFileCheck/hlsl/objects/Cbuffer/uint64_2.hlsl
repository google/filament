// RUN: %dxilver 1.1 | %dxc -E main -T cs_6_0 -validator-version 1.1 %s | FileCheck %s

// CHECK: 64-Bit integer
// CHECK: dx.op.bufferStore.i32
// CHECK: dx.op.bufferStore.i32
// CHECK: !{i32 0, i64 1048576

// Note: a change in the internal layout will produce
// a difference in the serialized flags, eg:
//
// 0n0532480 = 0b000010000010000000000000
// 0n1056768 = 0b000100000010000000000000
//
// This becomes part of the DXIL spec once finalized.

RWBuffer<uint64_t> buf;
uint64_t ref;

[numthreads(64,1,1)]
void main(uint tid : SV_DispatchThreadID)
{
  if (ref > buf[tid])
    buf[tid] = ref;
}