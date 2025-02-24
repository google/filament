//
// fragment_main
//
RWTexture1D<float4> arg_0 : register(u0, space1);

void textureStore_74886f() {
  arg_0[1] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_74886f();
  return;
}
//
// compute_main
//
RWTexture1D<float4> arg_0 : register(u0, space1);

void textureStore_74886f() {
  arg_0[1] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_74886f();
  return;
}
