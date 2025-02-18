struct main_outputs {
  float4 tint_symbol : SV_Target0;
};


float4 main_inner() {
  return float4(0.10000000149011611938f, 0.20000000298023223877f, 0.30000001192092895508f, 0.40000000596046447754f);
}

main_outputs main() {
  main_outputs v = {main_inner()};
  return v;
}

