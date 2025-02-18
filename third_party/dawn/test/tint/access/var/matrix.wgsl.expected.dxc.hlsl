RWByteAddressBuffer s : register(u0);

[numthreads(1, 1, 1)]
void main() {
  float3x3 m = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3 v = m[1];
  float f = v[1];
  s.Store(0u, asuint(f));
  return;
}
