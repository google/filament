//
// fragment_main
//
RWTexture2DArray<float4> arg_0 : register(u0, space1);

void textureStore_60975f() {
  arg_0[int3((1).xx, 1)] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_60975f();
  return;
}
//
// compute_main
//
RWTexture2DArray<float4> arg_0 : register(u0, space1);

void textureStore_60975f() {
  arg_0[int3((1).xx, 1)] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_60975f();
  return;
}
