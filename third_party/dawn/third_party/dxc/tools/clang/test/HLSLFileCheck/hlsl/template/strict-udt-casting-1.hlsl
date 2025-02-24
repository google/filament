// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 -HV 2021 -D USE_TEMPLATE %s | FileCheck %s

// Based on GitHub issue #3563

// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 2.000000e+00)

struct LinearRGB
{
  float3 RGB;
};

struct LinearYCoCg
{
  float3 YCoCg;
};

#ifndef USE_TEMPLATE

// Without -strict-udt-casting
// error: call to 'GetLuma4' is ambiguous
float GetLuma4(LinearRGB In)
{
  return In.RGB.g * 2.0 + In.RGB.r + In.RGB.b;
}

float GetLuma4(LinearYCoCg In)
{
  return In.YCoCg.x;
}

#else // works but code getting too complicated
  
template<typename InputColorSpace>
float GetLuma4(InputColorSpace In);

template<>
float GetLuma4<LinearRGB>(LinearRGB In)
{
  return In.RGB.g * 2.0 + In.RGB.r + In.RGB.b;
}

template<>
float GetLuma4<LinearYCoCg>(LinearYCoCg In)
{
  return In.YCoCg.x;
}

#endif

float4 main() : SV_TARGET {
  LinearYCoCg YCoCg = { {1.0f, 0.0, 0.0} };
  float f = GetLuma4(YCoCg);  // gets .x (1.0f)

  LinearRGB Color;
  Color.RGB = float3(0, f, 0);

  return float4(GetLuma4(Color), 0.0, 0.0, 0.0);
}
