// This file has unit tests to validate HLSL intrinsic "frac" when applied on literal floats.
// Looking at the online doc on frac (https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-frac)
// it should always return a positive fractional part in the interval: [0,1)
// Also nvidia's documentation (https://developer.download.nvidia.com/cg/frac.html) on frac says: frac(v) = v - floor(v);
// fxc seems to adhere to Nvidia's definition of frac. That is, frac(-1231.12121) evaluates to 0.878790 (not 0.12121).

// RUN: %dxc -E main -T vs_6_0 %s | %FileCheck %s

RWBuffer<float> fbuf;
RWBuffer<float4> f4buf;

void main() {
	// const prop
	// CHECK: float 5.000000e-01
	float y = -1.5;
	fbuf[0] = frac(y);

	// scalar values
	// CHECK: float 0.000000e+00
	fbuf[1] = frac(0);

	// CHECK: float 0.000000e+00
	fbuf[2] = frac(-0);

	// scalar literal values values
	// CHECK: float 5.000000e-01
	fbuf[7] = frac(1.5);

	// CHECK: float 0x3FEFFFFE20000000
	fbuf[9] = frac(-.0000009);

	// CHECK: float 5.000000e-01
	fbuf[8] = frac(-1.5);

	// CHECK: float 0x3E9421F600000000
	fbuf[10] = frac(0.0000003);

	// CHECK: float 0x3EC0C6F7A0000000
	fbuf[11] = frac(100.000002);

	// CHECK: float 0x3FE880FEC0000000
	fbuf[12] = frac(-1787.23425352);

	// vector splat
	// CHECK: float 0x3FEFF7CEC0000000, float 0x3FEFF7CEC0000000, float 0x3FEFF7CEC0000000, float 0x3FEFF7CEC0000000
	f4buf[0] = frac(float4(-1.001, -1.001, -1.001, -1.001));

	// vector non-splat
	// CHECK: float 0x3FEFF7CEC0000000, float 0x3FCDB57000000000, float 0x3FEC1F0000000000, float 0x3F50640000000000
	f4buf[1] = frac(float4(-1.001, 20.2321, -1231.12121, 11.001));

	// matrix splat
	// CHECK: float 0x3FCD710000000000, float 0x3FCD710000000000, float 0x3FCD710000000000, float 0x3FCD710000000000
	float2x2 fmat_splat = { 421.23, 421.23, 421.23, 421.23 };
	fmat_splat = frac(fmat_splat);
	f4buf[2] = float4(fmat_splat[0][0], fmat_splat[0][1], fmat_splat[1][0], fmat_splat[1][1]) ;

	// matrix non-splat
	// CHECK: float 0x3F2A400000000000, float 0x3F53A92A40000000, float 0x3FCD710000000000, float 0.000000e+00
	float2x2 fmat = { -21.0002, -0.0012, 421.23, 1 };
	fmat = frac(fmat);
	f4buf[3] = float4(fmat[0][0], fmat[0][1], fmat[1][0], fmat[1][1]) ;
}
