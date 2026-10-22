// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -HV 2017 %s | FileCheck %s

// CHECK: error: unknown type name 'int16_t'
// CHECK: error: unknown type name 'int32_t'
// CHECK: error: unknown type name 'uint16_t'
// CHECK: error: unknown type name 'uint32_t'
// CHECK: error: unknown type name 'float16_t'
// CHECK: error: unknown type name 'float32_t'
// CHECK: error: unknown type name 'float64_t'

// int64_t/uint64_t already supported from 6.0

float4 main(float col : COL) : SV_TARGET
{
    int16_t i0;
    int32_t i1;
    uint16_t i2;
    uint32_t i3;
    float16_t f0;
    float32_t f1;
    float64_t f2;
    return col;
}
