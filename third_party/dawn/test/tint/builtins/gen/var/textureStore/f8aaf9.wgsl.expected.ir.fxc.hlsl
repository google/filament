//
// fragment_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_f8aaf9() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  int4 arg_3 = (int(1)).xxxx;
  int4 v = arg_3;
  arg_0[int3(arg_1, arg_2)] = v;
}

void fragment_main() {
  textureStore_f8aaf9();
}

//
// compute_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_f8aaf9() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  int4 arg_3 = (int(1)).xxxx;
  int4 v = arg_3;
  arg_0[int3(arg_1, arg_2)] = v;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_f8aaf9();
}

