// RUN: %dxc -E main -T vs_6_3 %s | FileCheck %s

// Regression test for GitHub #1973, where the value of literals
// were implicitly truncated to 32 bits if not suffixed with LL.

RWStructuredBuffer<uint64_t4> buf;
void main()
{
  // CHECK: call void @dx.op.rawBufferStore.i64
  // CHECK: i64 4294967296, i64 4294967296, i64 4294967296, i64 4294967296, i8 15
  buf[0] = uint64_t4(
    4294967296, // 2^32
    0x100000000, // 2^32
    040000000000, // 2^32
    0b100000000000000000000000000000000); // 2^32
}