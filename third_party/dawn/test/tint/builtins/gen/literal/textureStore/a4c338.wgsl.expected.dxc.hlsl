//
// fragment_main
//
RWTexture1D<float4> arg_0 : register(u0, space1);

void textureStore_a4c338() {
  arg_0[1u] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_a4c338();
  return;
}
//
// compute_main
//
RWTexture1D<float4> arg_0 : register(u0, space1);

void textureStore_a4c338() {
  arg_0[1u] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_a4c338();
  return;
}
