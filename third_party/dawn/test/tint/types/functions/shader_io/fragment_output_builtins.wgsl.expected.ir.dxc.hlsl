//
// main1
//
struct main1_outputs {
  float tint_symbol : SV_Depth;
};


float main1_inner() {
  return 1.0f;
}

main1_outputs main1() {
  main1_outputs v = {main1_inner()};
  return v;
}

//
// main2
//
struct main2_outputs {
  uint tint_symbol : SV_Coverage;
};


uint main2_inner() {
  return 1u;
}

main2_outputs main2() {
  main2_outputs v = {main2_inner()};
  return v;
}

