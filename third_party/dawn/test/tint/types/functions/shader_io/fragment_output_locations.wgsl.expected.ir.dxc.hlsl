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

