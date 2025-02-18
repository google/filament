//
// fragment_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_e077e7() {
  int arg_1 = 1;
  int4 arg_2 = (1).xxxx;
  arg_0[arg_1] = arg_2;
}

void fragment_main() {
  textureStore_e077e7();
  return;
}
//
// compute_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_e077e7() {
  int arg_1 = 1;
  int4 arg_2 = (1).xxxx;
  arg_0[arg_1] = arg_2;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_e077e7();
  return;
}
