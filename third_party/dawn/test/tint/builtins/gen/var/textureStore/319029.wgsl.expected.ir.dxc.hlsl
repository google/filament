//
// fragment_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_319029() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  float4 arg_3 = (1.0f).xxxx;
  float4 v = arg_3;
  arg_0[int3(arg_1, arg_2)] = v;
}

void fragment_main() {
  textureStore_319029();
}

//
// compute_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_319029() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  float4 arg_3 = (1.0f).xxxx;
  float4 v = arg_3;
  arg_0[int3(arg_1, arg_2)] = v;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_319029();
}

