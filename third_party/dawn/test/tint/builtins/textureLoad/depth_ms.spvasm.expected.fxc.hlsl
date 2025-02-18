//
// vertex_main
//
Texture2DMS<float4> arg_0 : register(t0, space1);
static float4 tint_symbol_1 = (0.0f).xxxx;

void textureLoad_6273b1() {
  float res = 0.0f;
  res = float4(arg_0.Load(int3(0, 0, 0), 1).x, 0.0f, 0.0f, 0.0f).x;
  return;
}

void tint_symbol_2(float4 tint_symbol) {
  tint_symbol_1 = tint_symbol;
  return;
}

void vertex_main_1() {
  textureLoad_6273b1();
  tint_symbol_2((0.0f).xxxx);
  return;
}

struct vertex_main_out {
  float4 tint_symbol_1_1;
};
struct tint_symbol_3 {
  float4 tint_symbol_1_1 : SV_Position;
};

vertex_main_out vertex_main_inner() {
  vertex_main_1();
  vertex_main_out tint_symbol_4 = {tint_symbol_1};
  return tint_symbol_4;
}

tint_symbol_3 vertex_main() {
  vertex_main_out inner_result = vertex_main_inner();
  tint_symbol_3 wrapper_result = (tint_symbol_3)0;
  wrapper_result.tint_symbol_1_1 = inner_result.tint_symbol_1_1;
  return wrapper_result;
}
//
// fragment_main
//
Texture2DMS<float4> arg_0 : register(t0, space1);

void textureLoad_6273b1() {
  float res = 0.0f;
  res = float4(arg_0.Load(int3(0, 0, 0), 1).x, 0.0f, 0.0f, 0.0f).x;
  return;
}

void fragment_main_1() {
  textureLoad_6273b1();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}
//
// compute_main
//
Texture2DMS<float4> arg_0 : register(t0, space1);

void textureLoad_6273b1() {
  float res = 0.0f;
  res = float4(arg_0.Load(int3(0, 0, 0), 1).x, 0.0f, 0.0f, 0.0f).x;
  return;
}

void compute_main_1() {
  textureLoad_6273b1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
