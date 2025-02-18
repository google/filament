//
// fragment_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_65b6aa() {
  arg_0[1] = (1).xxxx;
}

void fragment_main() {
  textureStore_65b6aa();
  return;
}
//
// compute_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_65b6aa() {
  arg_0[1] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_65b6aa();
  return;
}
