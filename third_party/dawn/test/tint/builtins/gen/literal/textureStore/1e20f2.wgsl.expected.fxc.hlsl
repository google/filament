//
// fragment_main
//
RWTexture2D<float4> arg_0 : register(u0, space1);

void textureStore_1e20f2() {
  arg_0[(1u).xx] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_1e20f2();
  return;
}
//
// compute_main
//
RWTexture2D<float4> arg_0 : register(u0, space1);

void textureStore_1e20f2() {
  arg_0[(1u).xx] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_1e20f2();
  return;
}
