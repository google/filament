SKIP: INVALID

struct main0_outputs {
  int tint_symbol : SV_Target0;
};

struct main1_outputs {
  uint tint_symbol_1 : SV_Target1;
};

struct main2_outputs {
  float tint_symbol_2 : SV_Target2;
};

struct main3_outputs {
  float4 tint_symbol_3 : SV_Target3;
};

struct main4_outputs {
  float16_t tint_symbol_4 : SV_Target4;
};

struct main5_outputs {
  vector<float16_t, 3> tint_symbol_5 : SV_Target5;
};


int main0_inner() {
  return int(1);
}

uint main1_inner() {
  return 1u;
}

float main2_inner() {
  return 1.0f;
}

float4 main3_inner() {
  return float4(1.0f, 2.0f, 3.0f, 4.0f);
}

float16_t main4_inner() {
  return float16_t(2.25h);
}

vector<float16_t, 3> main5_inner() {
  return vector<float16_t, 3>(float16_t(3.0h), float16_t(5.0h), float16_t(8.0h));
}

main0_outputs main0() {
  main0_outputs v = {main0_inner()};
  return v;
}

main1_outputs main1() {
  main1_outputs v_1 = {main1_inner()};
  return v_1;
}

main2_outputs main2() {
  main2_outputs v_2 = {main2_inner()};
  return v_2;
}

main3_outputs main3() {
  main3_outputs v_3 = {main3_inner()};
  return v_3;
}

main4_outputs main4() {
  main4_outputs v_4 = {main4_inner()};
  return v_4;
}

main5_outputs main5() {
  main5_outputs v_5 = {main5_inner()};
  return v_5;
}

FXC validation failure:
<scrubbed_path>(18,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
