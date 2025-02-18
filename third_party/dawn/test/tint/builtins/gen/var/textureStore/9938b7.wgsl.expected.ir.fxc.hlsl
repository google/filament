//
// fragment_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_9938b7() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  int4 arg_3 = (int(1)).xxxx;
  int2 v = arg_1;
  int4 v_1 = arg_3;
  arg_0[int3(v, int(arg_2))] = v_1;
}

void fragment_main() {
  textureStore_9938b7();
}

//
// compute_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_9938b7() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  int4 arg_3 = (int(1)).xxxx;
  int2 v = arg_1;
  int4 v_1 = arg_3;
  arg_0[int3(v, int(arg_2))] = v_1;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_9938b7();
}

