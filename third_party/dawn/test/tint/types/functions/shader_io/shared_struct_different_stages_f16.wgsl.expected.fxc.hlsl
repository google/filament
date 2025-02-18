SKIP: INVALID

struct Interface {
  float col1;
  float16_t col2;
  float4 pos;
};
struct tint_symbol {
  float col1 : TEXCOORD1;
  float16_t col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

Interface vert_main_inner() {
  Interface tint_symbol_3 = {0.40000000596046447754f, float16_t(0.599609375h), (0.0f).xxxx};
  return tint_symbol_3;
}

tint_symbol vert_main() {
  Interface inner_result = vert_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_2 {
  float col1 : TEXCOORD1;
  float16_t col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

void frag_main_inner(Interface colors) {
  float r = colors.col1;
  float16_t g = colors.col2;
}

void frag_main(tint_symbol_2 tint_symbol_1) {
  Interface tint_symbol_4 = {tint_symbol_1.col1, tint_symbol_1.col2, float4(tint_symbol_1.pos.xyz, (1.0f / tint_symbol_1.pos.w))};
  frag_main_inner(tint_symbol_4);
  return;
}
FXC validation failure:
<scrubbed_path>(3,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
