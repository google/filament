// Rewrite unchanged result:
float fn() {
  float4 myvar = float4(1, 2, 3, 4);
  myvar.x = 1.F;
  myvar.y = 1.F;
  myvar.z = 1.F;
  myvar.w = 1.F;
  float4 myothervar;
  myothervar.rgba = myvar.xyzw;
  float f;
  f.x = 1;
  uint u;
  u = f.x;
  uint3 u3;
  u3.xyz = f.xxx;
  return f.x;
}


float4 plain(float4 param4) {
  return fn();
}


