// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -enable-16bit-types -HV 2018 %s  | FileCheck %s

struct MyStruct1
{
    half3   m_1;
    int4    m_2;
    half3   m_3;
    half4   m_4;
    double  m_5;
    half    m_6;
    half    m_7;
    half    m_8;
    int     m_9;
    int16_t m_10;
    uint16_t4 m_11;
};

struct MyStruct2
{
    double    m_1;
    half3     m_2;
    int       m_3;
    int16_t   m_4;
    float     m_5;
    uint16_t3 m_6;
    double    m_7;
};

RWStructuredBuffer<MyStruct1> g_sb1: register(u0);
RWStructuredBuffer<MyStruct2> g_sb2: register(u1);

float4 main() : SV_Target {
    // CHECK: call void @dx.op.rawBufferStore.f16(i32 140, %dx.types.Handle %g_sb1_UAV_structbuf, i32 0, i32 0, half 0xH3C00, half 0xH3C00, half 0xH3C00, half undef, i8 7, i32 2)
    // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %g_sb1_UAV_structbuf, i32 0, i32 8, i32 2, i32 2, i32 2, i32 2, i8 15, i32 4)
    // CHECK: call void @dx.op.rawBufferStore.f16(i32 140, %dx.types.Handle %g_sb1_UAV_structbuf, i32 0, i32 24, half 0xH4200, half 0xH4200, half 0xH4200, half undef, i8 7, i32 2)
    // CHECK: call void @dx.op.rawBufferStore.f16(i32 140, %dx.types.Handle %g_sb1_UAV_structbuf, i32 0, i32 30, half 0xH4400, half 0xH4400, half 0xH4400, half 0xH4400, i8 15, i32 2)
    // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %g_sb1_UAV_structbuf, i32 0, i32 40, i32 %{{.*}}, i32 %{{.*}}, i32 undef, i32 undef, i8 3, i32 8)
    // CHECK: call void @dx.op.rawBufferStore.f16(i32 140, %dx.types.Handle %g_sb1_UAV_structbuf, i32 0, i32 48, half 0xH4600, half undef, half undef, half undef, i8 1, i32 2)
    // CHECK: call void @dx.op.rawBufferStore.f16(i32 140, %dx.types.Handle %g_sb1_UAV_structbuf, i32 0, i32 50, half 0xH4700, half undef, half undef, half undef, i8 1, i32 2)
    // CHECK: call void @dx.op.rawBufferStore.f16(i32 140, %dx.types.Handle %g_sb1_UAV_structbuf, i32 0, i32 52, half 0xH4800, half undef, half undef, half undef, i8 1, i32 2)
    // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %g_sb1_UAV_structbuf, i32 0, i32 56, i32 9, i32 undef, i32 undef, i32 undef, i8 1, i32 4)
    // CHECK: call void @dx.op.rawBufferStore.i16(i32 140, %dx.types.Handle %g_sb1_UAV_structbuf, i32 0, i32 60, i16 10, i16 undef, i16 undef, i16 undef, i8 1, i32 2)
    // CHECK: call void @dx.op.rawBufferStore.i16(i32 140, %dx.types.Handle %g_sb1_UAV_structbuf, i32 0, i32 62, i16 11, i16 11, i16 11, i16 11, i8 15, i32 2)
    MyStruct1 myStruct;
    myStruct.m_1 = 1;
    myStruct.m_2 = 2;
    myStruct.m_3 = 3;
    myStruct.m_4 = 4;
    myStruct.m_5 = 5;
    myStruct.m_6 = 6;
    myStruct.m_7 = 7;
    myStruct.m_8 = 8;
    myStruct.m_9 = 9;
    myStruct.m_10 = 10;
    myStruct.m_11 = 11;
    g_sb1[0] = myStruct;

    // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %g_sb2_UAV_structbuf, i32 0, i32 0, i32 %{{.*}}, i32 %{{.*}}, i32 undef, i32 undef, i8 3, i32 8)
    // CHECK: call void @dx.op.rawBufferStore.f16(i32 140, %dx.types.Handle %g_sb2_UAV_structbuf, i32 0, i32 8, half 0xH4000, half 0xH4000, half 0xH4000, half undef, i8 7, i32 2)
    // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %g_sb2_UAV_structbuf, i32 0, i32 16, i32 3, i32 undef, i32 undef, i32 undef, i8 1, i32 4)
    // CHECK: call void @dx.op.rawBufferStore.i16(i32 140, %dx.types.Handle %g_sb2_UAV_structbuf, i32 0, i32 20, i16 4, i16 undef, i16 undef, i16 undef, i8 1, i32 2)
    // CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %g_sb2_UAV_structbuf, i32 0, i32 24, float 5.000000e+00, float undef, float undef, float undef, i8 1, i32 4)
    // CHECK: call void @dx.op.rawBufferStore.i16(i32 140, %dx.types.Handle %g_sb2_UAV_structbuf, i32 0, i32 28, i16 6, i16 6, i16 6, i16 undef, i8 7, i32 2)
    // CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %g_sb2_UAV_structbuf, i32 0, i32 40, i32 %{{.*}}, i32 %{{.*}}, i32 undef, i32 undef, i8 3, i32 8) 
    MyStruct2 myStruct2;
    myStruct2.m_1 = 1;
    myStruct2.m_2 = 2;
    myStruct2.m_3 = 3;
    myStruct2.m_4 = 4;
    myStruct2.m_5 = 5;
    myStruct2.m_6 = 6;
    myStruct2.m_7 = 7;
    g_sb2[0] = myStruct2;

    return 1;
}