struct Out {
  float4 pos;
  float none;
  float flat;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
};
struct tint_symbol {
  float none : TEXCOORD0;
  nointerpolation float flat : TEXCOORD1;
  linear float perspective_center : TEXCOORD2;
  linear centroid float perspective_centroid : TEXCOORD3;
  linear sample float perspective_sample : TEXCOORD4;
  noperspective float linear_center : TEXCOORD5;
  noperspective centroid float linear_centroid : TEXCOORD6;
  noperspective sample float linear_sample : TEXCOORD7;
  float4 pos : SV_Position;
};

Out main_inner() {
  Out tint_symbol_1 = (Out)0;
  return tint_symbol_1;
}

tint_symbol main() {
  Out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.none = inner_result.none;
  wrapper_result.flat = inner_result.flat;
  wrapper_result.perspective_center = inner_result.perspective_center;
  wrapper_result.perspective_centroid = inner_result.perspective_centroid;
  wrapper_result.perspective_sample = inner_result.perspective_sample;
  wrapper_result.linear_center = inner_result.linear_center;
  wrapper_result.linear_centroid = inner_result.linear_centroid;
  wrapper_result.linear_sample = inner_result.linear_sample;
  return wrapper_result;
}
