// RUN: %dxc -E main -T ps_6_0 -O0 %s | FileCheck %s

// CHECK: DerivCoarseX
// CHECK: DerivFineX
// CHECK: DerivCoarseY
// CHECK: FAbs
// CHECK: icmp slt
// CHECK: zext
// CHECK: fcmp fast olt
// CHECK: zext
// CHECK: Sqrt
// CHECK: Frc
// CHECK: Exp
// CHECK: Log
// CHECK: Round_z



float v;

float l;
float h;
float m;

float2 n;

uint i;

uint s;
uint2 a;
uint4 d;

int si;

float1x1 m1;
float2x2 m2;
float3x3 m3;
float4x4 m4;

float4 main(float4 arg : A, float4 arg1 :B) : SV_TARGET {

  float4 x = arg + arg1;
  x += ddx(x) + ddx_fine(x) + ddy_coarse(x) + fwidth(arg);
  x += sign(si) + sign(v);

  float t = radians(v); 
  t += determinant(m1);     
  t += determinant(m2);     
  t += determinant(m3);     
  t += determinant(m4);   
  t += degrees(v);           
  
  t += distance(arg, arg1); 
  
  t -= fmod(l,h); 
  
  t -= exp(v);

  t -= log(v);
 
  t -= log10(v);

  t += ldexp(l, m);


  float k;
  t += modf(v, k);
  t *= k;
   
 
  t += frexp(l, k);
  t -= k;

  return lit(l,h,m) + dst(arg, arg1) + t + x ;
}
