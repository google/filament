// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types %s | FileCheck %s

// Tests conversion between numerical types which happen as of
// constant initialization lists, since they use a different code path.

RWStructuredBuffer<bool4> buf_b;
RWStructuredBuffer<int4> buf_i;
RWStructuredBuffer<uint4> buf_u;
RWStructuredBuffer<float4> buf_f;

void main() {
    // To bool
    // CHECK: i32 1, i32 1, i32 1, i32 1, i8
    // CHECK: i32 1, i32 1, i32 1, i32 1, i8
    // CHECK: i32 1, i32 1, i32 0, i32 0, i8
    static const bool b[] = {
        true, // No-op
        (int)(-1), // icmp ne 0
        (uint)0xFFFFFFFF, // icmp ne 0
        (int16_t)(-1), // icmp ne 0
        (uint16_t)0xFFFF, // icmp ne 0
        (int64_t)(-1), // icmp ne 0
        (uint64_t)0xFFFFFFFFFFFFFFFF, // icmp ne 0
        (half)(-1.5f), // fcmp ne 0
        -1.5f, // fcmp ne 0
        (double)(-1.5f) }; // fcmp ne 0
    buf_b[0] = bool4(b[0], b[1], b[2], b[3]);
    buf_b[1] = bool4(b[4], b[5], b[6], b[7]);
    buf_b[2] = bool4(b[8], b[9], false, false);

    // To signed int
    // CHECK: i32 1, i32 -1, i32 -1, i32 -1, i8
    // CHECK: i32 65535, i32 -1, i32 -1, i32 -1, i8
    // CHECK: i32 -1, i32 -1, i32 0, i32 0, i8
    static const int i[] = {
        true, // ZExt
        (int)(-1), // No-op
        (uint)0xFFFFFFFF, // No-op (reinterpret)
        (int16_t)(-1), // SExt
        (uint16_t)0xFFFF, // ZExt
        (int64_t)(-1), // Trunc
        (uint64_t)0xFFFFFFFFFFFFFFFF, // Trunc
        (half)(-1.5f), // FPToSI
        -1.5f, // FPToSI
        (double)(-1.5f) }; // FPToSI
    buf_i[0] = int4(i[0], i[1], i[2], i[3]);
    buf_i[1] = int4(i[4], i[5], i[6], i[7]);
    buf_i[2] = int4(i[8], i[9], 0, 0);
    
    // To unsigned int
    // CHECK: i32 1, i32 -1, i32 -1, i32 -1, i8
    // CHECK: i32 65535, i32 -1, i32 -1, i32 0, i8
    // CHECK: i32 0, i32 0, i32 0, i32 0, i8
    static const uint u[] = {
        true, // ZExt
        (int)(-1), // No-op (reinterpret)
        (uint)0xFFFFFFFF, // No-op
        (int16_t)(-1), // SExt
        (uint16_t)0xFFFF, // ZExt
        (int64_t)(-1), // Trunc
        (uint64_t)0xFFFFFFFFFFFFFFFF, // Trunc
        (half)(-1.5f), // FPToUI
        -1.5f, // FPToUI
        (double)(-1.5f) }; // FPToUI
    buf_u[0] = uint4(u[0], u[1], u[2], u[3]);
    buf_u[1] = uint4(u[4], u[5], u[6], u[7]);
    buf_u[2] = uint4(u[8], u[9], 0, 0);
    
    // To float
    // CHECK: float 1.000000e+00, float -1.000000e+00, float 0x41F0000000000000, float -1.000000e+00, i8
    // CHECK: float 6.553500e+04, float -1.000000e+00, float 0x43F0000000000000, float -1.500000e+00, i8
    // CHECK: float -1.500000e+00, float -1.500000e+00, float 0.000000e+00, float 0.000000e+00, i8
    static const float f[] = {
        true, // UIToFP
        (int)(-1), // SIToFP
        (uint)0xFFFFFFFF, // UIToFP
        (int16_t)(-1), // SIToFP
        (uint16_t)0xFFFF, // UIToFP
        (int64_t)(-1), // SIToFP
        (uint64_t)0xFFFFFFFFFFFFFFFF, // UIToFP
        (half)(-1.5f), // FPExt
        -1.5f, // No-op
        (double)(-1.5f) }; // FPTrunc
    buf_f[0] = float4(f[0], f[1], f[2], f[3]);
    buf_f[1] = float4(f[4], f[5], f[6], f[7]);
    buf_f[2] = float4(f[8], f[9], 0, 0);
}