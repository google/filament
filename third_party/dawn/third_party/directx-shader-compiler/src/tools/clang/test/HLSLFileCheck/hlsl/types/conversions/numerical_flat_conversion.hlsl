// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types %s | FileCheck %s

// Tests conversion between numerical types which happen as part
// of a larger flat conversion between compound types.
// Assume that struct-to-struct is representative of all
// compound/numerical to compound/numerical flat casts.
// Assume that conversions between 16 and 32-bit types
// are representative of those between 16 and 64 or 32 and 64-bit types. 

struct B { bool value; };
struct I16 { int16_t value; };
struct U16 { uint16_t value; };
struct I32 { int value; };
struct U32 { uint value; };
struct F16 { half value; };
struct F32 { float value; };

RWStructuredBuffer<B> b;
RWStructuredBuffer<I16> i16;
RWStructuredBuffer<U16> u16;
RWStructuredBuffer<I32> i32;
RWStructuredBuffer<U32> u32;
RWStructuredBuffer<F16> f16;
RWStructuredBuffer<F32> f32;

void main() {
    int i = 0;
    int j = 0;

    // Integral casts
    // CHECK-NOT: zext
    // CHECK-NOT: sext
    // CHECK-NOT: trunc
    i32[i++] = (I32)u32[j++];
    u32[i++] = (U32)i32[j++];

    // CHECK: trunc
    // CHECK: trunc
    // CHECK: trunc
    // CHECK: trunc
    i16[i++] = (I16)i32[j++];
    i16[i++] = (I16)u32[j++];
    u16[i++] = (U16)i32[j++];
    u16[i++] = (U16)u32[j++];
    
    // CHECK: sext
    // CHECK: zext
    // CHECK: sext
    // CHECK: zext
    i32[i++] = (I32)i16[j++];
    i32[i++] = (I32)u16[j++];
    u32[i++] = (U32)i16[j++];
    u32[i++] = (U32)u16[j++];
    
    // Float casts
    // CHECK: fpext
    // CHECK: fptrunc
    f32[i++] = (F32)f16[j++];
    f16[i++] = (F16)f32[j++];
    
    // Integral/float casts
    // CHECK: fptosi
    // CHECK: fptoui
    // CHECK: sitofp
    // CHECK: uitofp
    i32[i++] = (I32)f32[j++];
    u32[i++] = (U32)f32[j++];
    f32[i++] = (F32)i32[j++];
    f32[i++] = (F32)u32[j++];

    // CHECK: fptosi
    // CHECK: fptoui
    // CHECK: sitofp
    // CHECK: uitofp
    i16[i++] = (I16)f32[j++];
    u16[i++] = (U16)f32[j++];
    f16[i++] = (F16)i32[j++];
    f16[i++] = (F16)u32[j++];
    
    // CHECK: fptosi
    // CHECK: fptoui
    // CHECK: sitofp
    // CHECK: uitofp
    i32[i++] = (I32)f16[j++];
    u32[i++] = (U32)f16[j++];
    f32[i++] = (F32)i16[j++];
    f32[i++] = (F32)u16[j++];

    // Casts to/from bool
    // CHECK: icmp ne
    // CHECK: icmp ne
    // CHECK: fcmp fast une
    b[i++] = (B)i32[j++];
    b[i++] = (B)u32[j++];
    b[i++] = (B)f32[j++];
    
    // CHECK: zext
    // CHECK: zext
    // CHECK: uitofp
    i32[i++] = (I32)b[j++];
    u32[i++] = (U32)b[j++];
    f32[i++] = (F32)b[j++];
}