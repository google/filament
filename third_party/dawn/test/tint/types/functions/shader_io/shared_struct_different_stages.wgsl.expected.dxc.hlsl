//
// vert_main
//
struct Interface {
  float col1;
  float col2;
  float4 pos;
};
struct tint_symbol {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

Interface vert_main_inner() {
  Interface tint_symbol_1 = {0.40000000596046447754f, 0.60000002384185791016f, (0.0f).xxxx};
  return tint_symbol_1;
}

tint_symbol vert_main() {
  Interface inner_result = vert_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
//
// frag_main
//
struct Interface {
  float col1;
  float col2;
  float4 pos;
};
struct tint_symbol_1 {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

void frag_main_inner(Interface colors) {
  float r = colors.col1;
  float g = colors.col2;
}

void frag_main(tint_symbol_1 tint_symbol) {
  Interface tint_symbol_2 = {tint_symbol.col1, tint_symbol.col2, float4(tint_symbol.pos.xyz, (1.0f / tint_symbol.pos.w))};
  frag_main_inner(tint_symbol_2);
  return;
}
