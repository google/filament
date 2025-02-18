struct main_outputs {
  precise float4 tint_symbol : SV_Position;
};


float4 main_inner() {
  return (0.0f).xxxx;
}

main_outputs main() {
  main_outputs v = {main_inner()};
  return v;
}

