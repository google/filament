struct main_outputs {
  float4 tint_symbol : SV_Target0;
};


void bar() {
}

float4 main_inner() {
  float2 a = (0.0f).xx;
  bar();
  return float4(0.40000000596046447754f, 0.40000000596046447754f, 0.80000001192092895508f, 1.0f);
}

main_outputs main() {
  main_outputs v = {main_inner()};
  return v;
}

