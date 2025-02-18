//
// fragment_main
//

RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_dde364() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  uint4 arg_3 = (1u).xxxx;
  uint4 v = arg_3;
  arg_0[int3(arg_1, arg_2)] = v;
}

void fragment_main() {
  textureStore_dde364();
}

//
// compute_main
//

RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_dde364() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  uint4 arg_3 = (1u).xxxx;
  uint4 v = arg_3;
  arg_0[int3(arg_1, arg_2)] = v;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_dde364();
}

