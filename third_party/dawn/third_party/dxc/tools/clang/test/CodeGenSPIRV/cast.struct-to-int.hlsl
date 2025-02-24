// RUN: %dxc -T cs_6_4 -HV 2021 -E main -fcgl  %s -spirv | FileCheck %s

struct ColorRGB { 
    uint R : 8;
    uint G : 8;
    uint B : 8;
};

struct ColorRGBA { 
    uint R : 8;
    uint G : 8;
    uint B : 8;
    uint A : 8;
};

struct TwoColors {
    ColorRGBA rgba1;
    ColorRGBA rgba2;
};

struct Mixed {
    float f;
    uint i;
};

struct Vectors {
    uint2 p1;
    uint2 p2;
};

RWStructuredBuffer<uint> buf : r0;
RWStructuredBuffer<uint64_t> lbuf : r1;
RWStructuredBuffer<Vectors> vbuf : r2;

// CHECK: OpName [[BUF:%[^ ]*]] "buf"
// CHECK: OpName [[LBUF:%[^ ]*]] "lbuf"
// CHECK: OpName [[COLORRGB:%[^ ]*]] "ColorRGB"
// CHECK: OpName [[COLORRGBA:%[^ ]*]] "ColorRGBA"
// CHECK: OpName [[TWOCOLORS:%[^ ]*]] "TwoColors"
// CHECK: OpName [[VECTORS:%[^ ]*]] "Vectors"
// CHECK: OpName [[MIXED:%[^ ]*]] "Mixed"

[numthreads(1,1,1)]
void main()
{
    ColorRGB rgb;
    ColorRGBA c0;
    ColorRGBA c1;
    TwoColors colors;
    Vectors v;
    Mixed m = {-1.0, 1};
    rgb.R = 127;
    rgb.G = 127;
    rgb.B = 127;
    c0.R = 255;
    c0.G = 127;
    c0.B = 63;
    c0.A = 31;
    c1.R = 15;
    c1.G = 7;
    c1.B = 3;
    c1.A = 1;
    colors.rgba1 = c0;
    colors.rgba2 = c1;
    v.p1.x = 3;
    v.p1.y = 2;
    v.p2.x = 1;
    v.p2.y = 0;

// CHECK-DAG: [[FLOAT:%[^ ]*]] = OpTypeFloat 32
// CHECK-DAG: [[FN1:%[^ ]*]] = OpConstant [[FLOAT]] -1
// CHECK-DAG: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U127:%[^ ]*]] = OpConstant [[UINT]] 127
// CHECK-DAG: [[INT:%[^ ]*]] = OpTypeInt 32 1
// CHECK-DAG: [[I0:%[^ ]*]] = OpConstant [[INT]] 0
// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[U8:%[^ ]*]] = OpConstant [[UINT]] 8
// CHECK-DAG: [[U255:%[^ ]*]] = OpConstant [[UINT]] 255
// CHECK-DAG: [[U3:%[^ ]*]] = OpConstant [[UINT]] 3
// CHECK-DAG: [[ULONG:%[^ ]*]] = OpTypeInt 64 0

    buf[0] = (uint) colors;
// CHECK: [[COLORS:%[^ ]*]] = OpLoad [[TWOCOLORS]]
// CHECK: [[COLORS0:%[^ ]*]] = OpCompositeExtract [[COLORRGBA]] [[COLORS]] 0
// CHECK: [[COLORS00:%[^ ]*]] = OpCompositeExtract [[UINT]] [[COLORS0]] 0
// CHECK: [[BUF00:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[BUF]] [[I0]] [[U0]]
// CHECK: OpStore [[BUF00]] [[COLORS00]]

    buf[0] -= (uint) rgb;
// CHECK: [[RGB:%[^ ]*]] = OpLoad [[COLORRGB]]
// CHECK: [[RGB0:%[^ ]*]] = OpCompositeExtract [[UINT]] [[RGB]] 0
// CHECK: [[BUF00_0:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[BUF]] [[I0]] [[U0]]
// CHECK: [[V1:%[^ ]*]] = OpLoad [[UINT]] [[BUF00_0]]
// CHECK: [[V2:%[^ ]*]] = OpISub [[UINT]] [[V1]] [[RGB0]]
// CHECK: OpStore [[BUF00_0]] [[V2]]

    lbuf[0] = (uint64_t) v;
// CHECK: [[VECS:%[^ ]*]] = OpLoad [[VECTORS]]
// CHECK: [[VECS0:%[^ ]*]] = OpCompositeExtract {{%v2uint}} [[VECS]] 0
// CHECK: [[VECS00:%[^ ]*]] = OpCompositeExtract [[UINT]] [[VECS0]] 0
// CHECK: [[V1_0:%[^ ]*]] = OpUConvert [[ULONG]] [[VECS00]]
// CHECK: [[LBUF00:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[LBUF]] [[I0]] [[U0]]
// CHECK: OpStore [[LBUF00]] [[V1_0]]

    lbuf[0] += (uint64_t) m;
// CHECK: [[MIX:%[^ ]*]] = OpLoad [[MIXED]]
// CHECK: [[MIX0:%[^ ]*]] = OpCompositeExtract [[FLOAT]] [[MIX]] 0
// CHECK: [[V2_0:%[^ ]*]] = OpConvertFToU [[ULONG]] [[MIX0]]
// CHECK: [[LBUF00_0:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[LBUF]] [[I0]] [[U0]]
// CHECK: [[V3:%[^ ]*]] = OpLoad [[ULONG]] [[LBUF00_0]]
// CHECK: [[V4:%[^ ]*]] = OpIAdd [[ULONG]] [[V3]] [[V2_0]]
// CHECK: OpStore [[LBUF00_0]] [[V4]]

    vbuf[0] = (Vectors) colors;
// CHECK: [[c0:%[^ ]*]] = OpLoad {{%[^ ]*}} %colors
// CHECK: [[c0_0:%[^ ]+]] = OpCompositeExtract %ColorRGBA [[c0]] 0
// The entire bit container extracted for each bitfield.
// CHECK: [[c0_0_0:%[^ ]*]] = OpCompositeExtract %uint [[c0_0]] 0
// CHECK: [[c0_0_1:%[^ ]*]] = OpCompositeExtract %uint [[c0_0]] 0
// CHECK: [[c0_0_2:%[^ ]*]] = OpCompositeExtract %uint [[c0_0]] 0
// CHECK: [[c0_0_3:%[^ ]*]] = OpCompositeExtract %uint [[c0_0]] 0
// CHECK: [[v0:%[^ ]*]] = OpCompositeConstruct %v2uint [[c0_0_0]] [[c0_0_1]]
// CHECK: [[v1:%[^ ]*]] = OpCompositeConstruct %v2uint [[c0_0_2]] [[c0_0_3]]
// CHECK: [[v:%[^ ]*]] = OpCompositeConstruct %Vectors_0 [[v0]] [[v1]]
// CHECK: [[vbuf:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %vbuf [[I0]] [[U0]]
// CHECK: [[v0:%[^ ]*]] = OpCompositeExtract %v2uint [[v]] 0
// CHECK: [[v1:%[^ ]*]] = OpCompositeExtract %v2uint [[v]] 1
// CHECK: [[v:%[^ ]*]] = OpCompositeConstruct %Vectors [[v0]] [[v1]]
// CHECK: OpStore [[vbuf]] [[v]]
}

