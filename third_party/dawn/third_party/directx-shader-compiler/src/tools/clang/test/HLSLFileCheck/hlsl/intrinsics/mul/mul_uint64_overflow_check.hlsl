// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Regression test for https://github.com/microsoft/DirectXShaderCompiler/issues/8598
//
// The 32x32->64 bit multiply overflow check below used to be folded by
// InstCombine into llvm.umul.with.overflow.i32, an intrinsic that is not legal
// in DXIL and failed validation with optimizations enabled. Make sure it
// compiles and validates, keeping a plain 64-bit multiply.

// CHECK-NOT: umul.with.overflow
// CHECK: mul nuw i64

RWStructuredBuffer<uint> buf : register(u0);

[numthreads(1, 1, 1)]
void main()
{
    uint x = buf[0];
    uint y = buf[1];
    if (((uint64_t)x) * ((uint64_t)y) > 0xffffffffull) {
        buf[2] = 1;
    }
}
