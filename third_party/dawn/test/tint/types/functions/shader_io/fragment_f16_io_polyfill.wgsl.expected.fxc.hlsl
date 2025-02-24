SKIP: INVALID

struct Outputs {
  float16_t a;
  vector<float16_t, 4> b;
};
struct tint_symbol_1 {
  float16_t loc1 : TEXCOORD1;
  vector<float16_t, 4> loc2 : TEXCOORD2;
};
struct tint_symbol_2 {
  float16_t a : SV_Target1;
  vector<float16_t, 4> b : SV_Target2;
};

Outputs frag_main_inner(float16_t loc1, vector<float16_t, 4> loc2) {
  Outputs tint_symbol_3 = {(loc1 * float16_t(2.0h)), (loc2 * float16_t(3.0h))};
  return tint_symbol_3;
}

tint_symbol_2 frag_main(tint_symbol_1 tint_symbol) {
  Outputs inner_result = frag_main_inner(tint_symbol.loc1, tint_symbol.loc2);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.a = inner_result.a;
  wrapper_result.b = inner_result.b;
  return wrapper_result;
}
FXC validation failure:
<scrubbed_path>(2,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
