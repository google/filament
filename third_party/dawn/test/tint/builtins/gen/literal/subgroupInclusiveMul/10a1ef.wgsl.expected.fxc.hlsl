SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupInclusiveMul_10a1ef() {
  float16_t res = (WavePrefixProduct(float16_t(1.0h)) * float16_t(1.0h));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupInclusiveMul_10a1ef());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupInclusiveMul_10a1ef());
  return;
}
