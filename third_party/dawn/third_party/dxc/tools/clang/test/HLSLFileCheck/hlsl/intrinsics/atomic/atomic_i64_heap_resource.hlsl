// RUN: %dxc -T ps_6_6 %s | FileCheck %s
// Test atomic operations on heap resources

// For now just check that it compiles.
// TODO: improve disassembly output for resources to verify the flag

// CHECK: Note: shader requires additional functionality:
// CHECK: 64-bit Atomics on Typed Resources

RWTexture2D<int64_t> myTexture : register (u0);

void main() : SV_Target
{
    InterlockedAdd(myTexture[int2(0,0)], 1);
}
