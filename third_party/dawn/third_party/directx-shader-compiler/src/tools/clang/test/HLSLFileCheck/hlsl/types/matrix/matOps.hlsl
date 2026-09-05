// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: sdiv

float1x1 f1x1;
float1x2 f1x2;
float1x3 f1x3;
float1x4 f1x4;

float2x1 f2x1;
float2x2 f2x2;
float2x3 f2x3;
float2x4 f2x4;

float3x1 f3x1;
float3x2 f3x2;
float3x3 f3x3;
float3x4 f3x4;

float4x1 f4x1;
float4x2 f4x2;
float4x3 f4x3;
float4x4 f4x4;
float4x4 f4x4b;

vector<float,1>  f1;
float2 f2;
float3 f3;
float4 f4;

int4x4 i4x4;
int4   i4;
float  x;
int    i;

struct T {
   float a;
   float b;
};
T t;
float4 main(float4 a : A) : SV_TARGET
{
  float4x2 mt2 = {t, t,t,t};
  float4x4 mt = x;
  mt += f4x4;
  int4x4 im = i;
  im[2] = 1;
  im |= ~(i4x4<<2) + i4x4>>2 % (im & 2 | im ^ i);
  im /= 3;
  bool4x4 b = (im++) < i;
  b = !b;
  float4 f4b = mul(f4, x+mt-x*f4x4b/im);
  return f4b;
}

