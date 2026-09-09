// RUN: %dxc -E main -T ps_6_0 -HV 2018 %s | FileCheck %s
// github issue #1725
// Test implicit cast scenario between structs of identical layout
// which would crash as reported by a user.

// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 0.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 0.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 0.000000e+00)
  
struct BUFF
{
    float4 rt0 : SV_Target0;
};

struct VSOUT_A
{
    float4 hclip : SV_Position;
    float3 wp : WORLD_POSITION;
};

struct VSOUT_B
{
    VSOUT_A position;
    float3 normal : NORMAL;
};

struct VSOUT_C
{
    VSOUT_A position;
    float3 normal : NORMAL;
};

float3 getNormal(in VSOUT_B vsout)
{
    return vsout.normal;
}

struct VSOUT_D
{
    VSOUT_C standard;
};

void foo(in VSOUT_A stdP)
{
    (stdP) = (stdP);
}

BUFF main(VSOUT_D input)
{
    foo(input.standard.position);        // comment out and it will compile fine
    float3 N = getNormal( input.standard ); // comment this out and it will compile fine    
    return (BUFF)0;
}
