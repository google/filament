RWByteAddressBuffer s : register(u0);

[numthreads(1, 1, 1)]
void main() {
  float3 v = float3(0.0f, 0.0f, 0.0f);
  float scalar = v.y;
  float2 swizzle2 = v.xz;
  float3 swizzle3 = v.xzy;
  s.Store3(0u, asuint(((float3((scalar).xxx) + float3(swizzle2, 1.0f)) + swizzle3)));
  return;
}
