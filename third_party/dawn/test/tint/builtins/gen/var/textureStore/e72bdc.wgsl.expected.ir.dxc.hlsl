//
// fragment_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_e72bdc() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  float4 arg_3 = (1.0f).xxxx;
  int2 v = arg_1;
  float4 v_1 = arg_3;
  arg_0[int3(v, int(arg_2))] = v_1;
}

void fragment_main() {
  textureStore_e72bdc();
}

//
// compute_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_e72bdc() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  float4 arg_3 = (1.0f).xxxx;
  int2 v = arg_1;
  float4 v_1 = arg_3;
  arg_0[int3(v, int(arg_2))] = v_1;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_e72bdc();
}

