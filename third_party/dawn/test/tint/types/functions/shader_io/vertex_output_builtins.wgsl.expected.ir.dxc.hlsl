struct main_outputs {
  float4 tint_symbol : SV_Position;
};


float4 main_inner() {
  return float4(1.0f, 2.0f, 3.0f, 4.0f);
}

main_outputs main() {
  main_outputs v = {main_inner()};
  return v;
}

