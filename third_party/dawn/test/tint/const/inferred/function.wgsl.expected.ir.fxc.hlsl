struct main_outputs {
  float4 tint_symbol : SV_Target0;
};


float4 main_inner() {
  return (0.0f).xxxx;
}

main_outputs main() {
  main_outputs v = {main_inner()};
  return v;
}

