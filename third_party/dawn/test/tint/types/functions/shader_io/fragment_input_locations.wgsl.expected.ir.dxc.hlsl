struct main_inputs {
  nointerpolation int loc0 : TEXCOORD0;
  nointerpolation uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
};


void main_inner(int loc0, uint loc1, float loc2, float4 loc3) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  float4 v = loc3;
}

void main(main_inputs inputs) {
  main_inner(inputs.loc0, inputs.loc1, inputs.loc2, inputs.loc3);
}

