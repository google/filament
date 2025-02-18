//
// vert_main1
//
struct VertexOutput {
  float4 pos;
  int loc0;
};

VertexOutput foo(float x) {
  VertexOutput tint_symbol_1 = {float4(x, x, x, 1.0f), 42};
  return tint_symbol_1;
}

struct tint_symbol {
  nointerpolation int loc0 : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vert_main1_inner() {
  return foo(0.5f);
}

tint_symbol vert_main1() {
  VertexOutput inner_result = vert_main1_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.loc0 = inner_result.loc0;
  return wrapper_result;
}
//
// vert_main2
//
struct VertexOutput {
  float4 pos;
  int loc0;
};

VertexOutput foo(float x) {
  VertexOutput tint_symbol_1 = {float4(x, x, x, 1.0f), 42};
  return tint_symbol_1;
}

struct tint_symbol {
  nointerpolation int loc0 : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vert_main2_inner() {
  return foo(0.25f);
}

tint_symbol vert_main2() {
  VertexOutput inner_result = vert_main2_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.loc0 = inner_result.loc0;
  return wrapper_result;
}
