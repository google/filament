#version 310 es

uvec3 v = uvec3(0u);
layout(binding = 2, std430)
buffer ResultMatrix_1_ssbo {
  float numbers[];
} resultMatrix;
float binaryOperation_f1_f1_(inout float a, inout float b) {
  float x_26 = 0.0f;
  if ((b == 0.0f)) {
    return 1.0f;
  }
  float x_21 = b;
  if (!((round((x_21 - (2.0f * floor((x_21 / 2.0f))))) == 1.0f))) {
    x_26 = pow(abs(a), b);
  } else {
    x_26 = (sign(a) * pow(abs(a), b));
  }
  float x_41 = x_26;
  return x_41;
}
void main_1() {
  int index = 0;
  int a_1 = 0;
  float param = 0.0f;
  float param_1 = 0.0f;
  index = int(v.x);
  a_1 = -10;
  int x_63 = index;
  param = -4.0f;
  param_1 = -3.0f;
  float x_68 = binaryOperation_f1_f1_(param, param_1);
  uint v_1 = (uint(resultMatrix.numbers.length()) - 1u);
  uint v_2 = min(uint(x_63), v_1);
  resultMatrix.numbers[v_2] = x_68;
}
void main_inner(uvec3 v_3) {
  v = v_3;
  main_1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
