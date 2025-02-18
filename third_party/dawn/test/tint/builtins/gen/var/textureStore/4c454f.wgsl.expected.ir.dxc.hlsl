//
// fragment_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_4c454f() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  int4 arg_3 = (int(1)).xxxx;
  int4 v = arg_3;
  arg_0[uint3(arg_1, arg_2)] = v;
}

void fragment_main() {
  textureStore_4c454f();
}

//
// compute_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_4c454f() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  int4 arg_3 = (int(1)).xxxx;
  int4 v = arg_3;
  arg_0[uint3(arg_1, arg_2)] = v;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_4c454f();
}

