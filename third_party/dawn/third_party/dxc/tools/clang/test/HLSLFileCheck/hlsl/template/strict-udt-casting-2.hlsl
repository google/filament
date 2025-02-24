// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s

// Based on GitHub issue #3564

// CHECK: error: cannot initialize a variable of type 'LinearYCoCg' with an rvalue of type 'LinearRGB'

struct LinearRGB
{
  float3 RGB;
};

struct LinearYCoCg
{
  float3 YCoCg;
};


LinearYCoCg RGBToYCoCg(LinearRGB In)
{
  float Y = dot(In.RGB, float3(1, 2, 1));
  float Co = dot(In.RGB, float3(2, 0, -2));
  float Cg = dot(In.RGB, float3(-1, 2, -1));

  LinearYCoCg Out;
  Out.YCoCg = float3(Y, Co, Cg);
  return Out;
}

LinearRGB YCoCgToRGB(LinearYCoCg In)
{
  float Y =  In.YCoCg.x * float(0.25);
  float Co = In.YCoCg.y * float(0.25);
  float Cg = In.YCoCg.z * float(0.25);

  float R = Y + Co - Cg;
  float G = Y + Cg;
  float B = Y - Co - Cg;

  LinearRGB Out;
  Out.RGB = float3(R, G, B);
  return Out;
}

template<typename OutputColorSpace, typename InputColorSpace>
OutputColorSpace TransformColor(InputColorSpace In)
{
  //static_assert(sizeof(InputColorSpace) == 0, "TransformColor() not implemented");
  float Array[!sizeof(InputColorSpace)];
  Array[0];
  return In;
}

template<>
LinearYCoCg TransformColor<LinearYCoCg, LinearRGB>(LinearRGB In)
{
  return RGBToYCoCg(In);
}


template<>
LinearRGB TransformColor<LinearRGB, LinearYCoCg>(LinearYCoCg In)
{
  return YCoCgToRGB(In);
}

float4 main() : SV_TARGET {
  LinearRGB Color;
  Color.RGB = float3(0, 1, 0);

  LinearYCoCg Color2 = TransformColor<LinearYCoCg>(Color);
  LinearRGB Color3 = TransformColor<LinearRGB>(Color2);
  
  // ISSUE: TransformColor<LinearRGB>() returns LinearRGB, but get cast silently in LinearYCoCg without error.
  LinearYCoCg Color4 = TransformColor<LinearRGB>(Color2);

  return float4(Color4.YCoCg, 0.0);
}
