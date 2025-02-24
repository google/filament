Texture2DMS<float4> texture0 : register(t0);

RWByteAddressBuffer results : register(u2);

[numthreads(1, 1, 1)]
void main() {
  results.Store(0u, asuint(texture0.Load(int2((0).xx), 0).x));
  return;
}
