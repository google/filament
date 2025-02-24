#version 310 es
precision highp float;
precision highp int;

float v(int v_1) {
  return float(v_1);
}
void main() {
  int v_2 = 0;
  float v_3 = v(v_2);
}
