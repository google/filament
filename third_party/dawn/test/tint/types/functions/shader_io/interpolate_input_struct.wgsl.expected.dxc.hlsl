struct In {
  float none;
  float flat;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
  float perspective_default;
  float linear_default;
};
struct tint_symbol_2 {
  float none : TEXCOORD0;
  nointerpolation float flat : TEXCOORD1;
  linear float perspective_center : TEXCOORD2;
  linear centroid float perspective_centroid : TEXCOORD3;
  linear sample float perspective_sample : TEXCOORD4;
  noperspective float linear_center : TEXCOORD5;
  noperspective centroid float linear_centroid : TEXCOORD6;
  noperspective sample float linear_sample : TEXCOORD7;
  linear float perspective_default : TEXCOORD8;
  noperspective float linear_default : TEXCOORD9;
};

void main_inner(In tint_symbol) {
}

void main(tint_symbol_2 tint_symbol_1) {
  In tint_symbol_3 = {tint_symbol_1.none, tint_symbol_1.flat, tint_symbol_1.perspective_center, tint_symbol_1.perspective_centroid, tint_symbol_1.perspective_sample, tint_symbol_1.linear_center, tint_symbol_1.linear_centroid, tint_symbol_1.linear_sample, tint_symbol_1.perspective_default, tint_symbol_1.linear_default};
  main_inner(tint_symbol_3);
  return;
}
