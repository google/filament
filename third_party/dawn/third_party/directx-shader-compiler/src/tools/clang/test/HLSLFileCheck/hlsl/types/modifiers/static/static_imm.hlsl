// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure create constant for static array.
// CHECK: constant [3 x float] [float 0.000000e+00, float 0x3FF1B22D20000000, float -2.800000e+01]
// CHECK: constant [3 x float] [float 0x3FF4A3D700000000, float 0x4046666660000000, float 0x3FB99999A0000000]

static const float2 t[ 3 ]=
	{

	
	float2(0, 3.0f) * 0.43f,
		
        float2(1.58f, 64) * 0.7f,
		
         float2(-28, 0.1f)
	};

uint i;

float2 main() : SV_Target {
  return t[i];
}