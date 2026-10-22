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



min16float v;

min16float l;
min16float h;
min16float m;

min16float2 n;

uint i;

uint s;
uint2 a;
uint4 d;

int si;

min16float1x1 m1;
min16float2x2 m2;
min16float3x3 m3;
min16float4x4 m4;

float4 main(float4 arg : A, float4 arg1 :B) : SV_TARGET {

  min16float4 x = arg + arg1;
  x += ddx(x) + ddx_fine(x) + ddy_coarse(x) + fwidth(arg);
  x += sign(si) + sign(v);

  min16float t = radians(v); 
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


  min16float k;
  t += modf(v, k);
  t *= k;
   
 
  t += frexp(l, k);
  t -= k;

  return lit(l,h,m) + dst(arg, arg1) + t + x ;
}
