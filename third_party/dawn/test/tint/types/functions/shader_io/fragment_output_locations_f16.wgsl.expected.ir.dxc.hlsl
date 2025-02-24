//
// main0
//
struct main0_outputs {
  int tint_symbol : SV_Target0;
};


int main0_inner() {
  return int(1);
}

main0_outputs main0() {
  main0_outputs v = {main0_inner()};
  return v;
}

//
// main1
//
struct main1_outputs {
  uint tint_symbol : SV_Target1;
};


uint main1_inner() {
  return 1u;
}

main1_outputs main1() {
  main1_outputs v = {main1_inner()};
  return v;
}

//
// main2
//
struct main2_outputs {
  float tint_symbol : SV_Target2;
};


float main2_inner() {
  return 1.0f;
}

main2_outputs main2() {
  main2_outputs v = {main2_inner()};
  return v;
}

//
// main3
//
struct main3_outputs {
  float4 tint_symbol : SV_Target3;
};


float4 main3_inner() {
  return float4(1.0f, 2.0f, 3.0f, 4.0f);
}

main3_outputs main3() {
  main3_outputs v = {main3_inner()};
  return v;
}

//
// main4
//
struct main4_outputs {
  float16_t tint_symbol : SV_Target4;
};


float16_t main4_inner() {
  return float16_t(2.25h);
}

main4_outputs main4() {
  main4_outputs v = {main4_inner()};
  return v;
}

//
// main5
//
struct main5_outputs {
  vector<float16_t, 3> tint_symbol : SV_Target5;
};


vector<float16_t, 3> main5_inner() {
  return vector<float16_t, 3>(float16_t(3.0h), float16_t(5.0h), float16_t(8.0h));
}

main5_outputs main5() {
  main5_outputs v = {main5_inner()};
  return v;
}

