// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: renderTargetGetSampleCount
// CHECK: renderTargetGetSamplePosition
// CHECK: evalCentroid
// CHECK: evalSampleIndex
// CHECK: evalSnapped
// CHECK: checkAccessFullyMapped
// CHECK: calculateLOD
// CHECK: i1 true
// CHECK: calculateLOD
// CHECK: i1 false
// CHECK: texture2DMSGetSamplePosition
// CHECK: getDimensions

SamplerState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);
Texture2DMS<float> srv1 : register(t6);
Texture2DMSArray<float> srv2 : register(t7);

uint mipLevel;

struct Foo
{
  float2 a;
  float3 b;
  int2 c[4];
};

RWStructuredBuffer<Foo> buf2;
RWBuffer<float4> buf1;


float4 main(float4 arg : A, noperspective float4 arg1 : B, centroid float4 arg2 : C,
            sample float4 arg3 : D
 ) : SV_TARGET {
  int sampleIdx = GetRenderTargetSampleCount()-1;
  float2 samplePos = GetRenderTargetSamplePosition(sampleIdx);

  float4 t = EvaluateAttributeCentroid(arg);
  t += EvaluateAttributeAtSample(arg, sampleIdx);
  t += EvaluateAttributeSnapped(arg, int2(1,2));

  uint status;
  float4 d = text1.Sample( samp1, samplePos, int2(1,1), 1.2, status );
  if (CheckAccessFullyMapped(status))
    t += d;


  t += text1.CalculateLevelOfDetail(samp1, samplePos);
  t += text1.CalculateLevelOfDetailUnclamped(samp1, samplePos);

  float2 samplePos2 = srv1.GetSamplePosition(sampleIdx);
  d = srv1.Load(samplePos2, sampleIdx);
  t += d;

  uint width;
  uint height;
  uint numOfLevels;
  text1.GetDimensions(mipLevel, width, height, numOfLevels);
  t += width + height + numOfLevels;

  text1.GetDimensions(width, height);
  t += width + height;

  uint arraySize;
  uint numSamples;
  srv2.GetDimensions(width, height, arraySize, numSamples);
  t += width + height + numSamples + arraySize;
   
  uint numStructs;
  uint stride;
  buf2.GetDimensions(numStructs, stride);
  t += numStructs + stride;  

  uint dim;
  buf1.GetDimensions(dim);
  t += dim;
  return t;
}
