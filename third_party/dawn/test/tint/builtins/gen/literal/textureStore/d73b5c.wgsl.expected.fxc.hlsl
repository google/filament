//
// fragment_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_d73b5c() {
  arg_0[1] = (1).xxxx;
}

void fragment_main() {
  textureStore_d73b5c();
  return;
}
//
// compute_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_d73b5c() {
  arg_0[1] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_d73b5c();
  return;
}
