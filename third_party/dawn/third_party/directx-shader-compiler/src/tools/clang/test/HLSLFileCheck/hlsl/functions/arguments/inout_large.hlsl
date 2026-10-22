// RUN: %dxc -T lib_6_3 %s | FileCheck %s

// Large struct with a lot of members to overwhelm the default memory data analysis lookback
struct BigusStructus
{
    float  BigusArrayus[100];
};

// CHECK: getelementptr inbounds %struct.BigusStructus, %struct.BigusStructus* %bs, i32 0, i32 0, i32 1
// CHECK: load float, float*
// CHECK: fmul fast float
// CHECK: insertelement <4 x float> undef, float
// CHECK: insertelement <4 x float>
// CHECK: insertelement <4 x float>
// CHECK: insertelement <4 x float>
// CHECK: ret <4 x float>

export
float4 AccessJustOneStruct(inout BigusStructus bs)
{
   return float4(bs.BigusArrayus[1]*255, 1, 0, 0);
}

// CHECK: getelementptr inbounds [100 x float], [100 x float]* %ba, i32 0, i32 1
// CHECK: load float, float*
// CHECK: fmul fast float
// CHECK: insertelement <4 x float> undef, float
// CHECK: insertelement <4 x float>
// CHECK: insertelement <4 x float>
// CHECK: insertelement <4 x float>
// CHECK: ret <4 x float>
export
float4 AccessJustOneArray(inout float ba[100])
{
   return float4(ba[1]*255, 1, 0, 0);
}
