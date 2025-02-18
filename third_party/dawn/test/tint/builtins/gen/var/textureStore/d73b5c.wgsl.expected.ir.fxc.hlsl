//
// fragment_main
//

RWTexture1D<int4> arg_0 : register(u0, space1);
void textureStore_d73b5c() {
  int arg_1 = int(1);
  int4 arg_2 = (int(1)).xxxx;
  arg_0[arg_1] = arg_2;
}

void fragment_main() {
  textureStore_d73b5c();
}

//
// compute_main
//

RWTexture1D<int4> arg_0 : register(u0, space1);
void textureStore_d73b5c() {
  int arg_1 = int(1);
  int4 arg_2 = (int(1)).xxxx;
  arg_0[arg_1] = arg_2;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_d73b5c();
}

