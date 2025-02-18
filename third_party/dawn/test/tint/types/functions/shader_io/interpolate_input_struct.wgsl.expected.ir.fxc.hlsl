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

struct main_inputs {
  float In_none : TEXCOORD0;
  nointerpolation float In_flat : TEXCOORD1;
  linear float In_perspective_center : TEXCOORD2;
  linear centroid float In_perspective_centroid : TEXCOORD3;
  linear sample float In_perspective_sample : TEXCOORD4;
  noperspective float In_linear_center : TEXCOORD5;
  noperspective centroid float In_linear_centroid : TEXCOORD6;
  noperspective sample float In_linear_sample : TEXCOORD7;
  linear float In_perspective_default : TEXCOORD8;
  noperspective float In_linear_default : TEXCOORD9;
};


void main_inner(In v) {
}

void main(main_inputs inputs) {
  In v_1 = {inputs.In_none, inputs.In_flat, inputs.In_perspective_center, inputs.In_perspective_centroid, inputs.In_perspective_sample, inputs.In_linear_center, inputs.In_linear_centroid, inputs.In_linear_sample, inputs.In_perspective_default, inputs.In_linear_default};
  main_inner(v_1);
}

