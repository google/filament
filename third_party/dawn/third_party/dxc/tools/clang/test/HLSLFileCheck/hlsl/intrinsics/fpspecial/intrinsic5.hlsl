// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK-NOT: Round_ne
// CHECK: fptosi
// CHECK: IsInf
// CHECK: IsNaN
// CHECK: IsFinite
// CHECK: dot4
// CHECK: Fma
// CHECK: dot4
// CHECK: Sqrt
// CHECK: FMad
// CHECK: IMad
// CHECK: UMad


float4 n;
float4 i;
float4 ng;

float eta;

double a;
double b;
double c;

float x;
float y;
float z;

int ix;
int iy;
int iz;

uint ux;
uint uy;
uint uz;


float4 main(float4 arg : A) : SV_TARGET {
  int4 i4 = D3DCOLORtoUBYTE4(arg);
  if (any(i4) || all(i4))
    return i4;

  bool4 inf = isinf(arg);
  bool4 nan = isnan(arg);
  bool4 isf = isfinite(arg);

  if (any(inf))
    return inf;
  if (all(nan))
    return nan;
  if (any(isf))
    return isf;

  float4 ff = faceforward(n, i, ng);
  double ma = fma(a, b, c);
  float st = step(y, x) + smoothstep(y, x, 0.3);
  float4 ref = refract(i, n, eta);
  float fmad = mad(x,y,z);
  int   ima = mad(ix,iy,iz);
  uint  uma = mad(ux,uy,uz);
  return ff + ma + st + ref + fmad + ima + uma;
}
