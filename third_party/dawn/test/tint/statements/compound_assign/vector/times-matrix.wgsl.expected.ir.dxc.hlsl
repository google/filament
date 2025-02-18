
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store4(0u, asuint(mul(float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx), asfloat(v.Load4(0u)))));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

