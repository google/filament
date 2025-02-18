struct VSOut {
  float4 pos;
};

struct main_outputs {
  float4 VSOut_pos : SV_Position;
};


void foo(inout VSOut v) {
  float4 pos = float4(1.0f, 2.0f, 3.0f, 4.0f);
  v.pos = pos;
}

VSOut main_inner() {
  VSOut v_1 = (VSOut)0;
  foo(v_1);
  VSOut v_2 = v_1;
  return v_2;
}

main_outputs main() {
  VSOut v_3 = main_inner();
  main_outputs v_4 = {v_3.pos};
  return v_4;
}

