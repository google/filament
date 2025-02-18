struct Outputs {
  float16_t a;
  vector<float16_t, 4> b;
};

struct frag_main_outputs {
  float16_t Outputs_a : SV_Target1;
  vector<float16_t, 4> Outputs_b : SV_Target2;
};

struct frag_main_inputs {
  float16_t loc1 : TEXCOORD1;
  vector<float16_t, 4> loc2 : TEXCOORD2;
};


Outputs frag_main_inner(float16_t loc1, vector<float16_t, 4> loc2) {
  Outputs v = {(loc1 * float16_t(2.0h)), (loc2 * float16_t(3.0h))};
  return v;
}

frag_main_outputs frag_main(frag_main_inputs inputs) {
  Outputs v_1 = frag_main_inner(inputs.loc1, inputs.loc2);
  frag_main_outputs v_2 = {v_1.a, v_1.b};
  return v_2;
}

