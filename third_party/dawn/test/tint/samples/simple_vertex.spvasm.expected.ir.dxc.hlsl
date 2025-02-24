struct main_out {
  float4 gl_Position;
};

struct main_outputs {
  float4 main_out_gl_Position : SV_Position;
};


static float4 gl_Position = (0.0f).xxxx;
void main_1() {
  gl_Position = (0.0f).xxxx;
}

main_out main_inner() {
  main_1();
  main_out v = {gl_Position};
  return v;
}

main_outputs main() {
  main_out v_1 = main_inner();
  main_outputs v_2 = {v_1.gl_Position};
  return v_2;
}

