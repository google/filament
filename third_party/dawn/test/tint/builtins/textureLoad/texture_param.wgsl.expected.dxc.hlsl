//
// vertex_main
//
Texture2D<int4> arg_0 : register(t0, space1);

int4 textureLoad2d(Texture2D<int4> tint_symbol, int2 coords, int level) {
  return tint_symbol.Load(int3(coords, level));
}

void doTextureLoad() {
  int4 res = textureLoad2d(arg_0, (0).xx, 0);
}

struct tint_symbol_1 {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  doTextureLoad();
  return (0.0f).xxxx;
}

tint_symbol_1 vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
//
// fragment_main
//
Texture2D<int4> arg_0 : register(t0, space1);

int4 textureLoad2d(Texture2D<int4> tint_symbol, int2 coords, int level) {
  return tint_symbol.Load(int3(coords, level));
}

void doTextureLoad() {
  int4 res = textureLoad2d(arg_0, (0).xx, 0);
}

void fragment_main() {
  doTextureLoad();
  return;
}
//
// compute_main
//
Texture2D<int4> arg_0 : register(t0, space1);

int4 textureLoad2d(Texture2D<int4> tint_symbol, int2 coords, int level) {
  return tint_symbol.Load(int3(coords, level));
}

void doTextureLoad() {
  int4 res = textureLoad2d(arg_0, (0).xx, 0);
}

[numthreads(1, 1, 1)]
void compute_main() {
  doTextureLoad();
  return;
}
