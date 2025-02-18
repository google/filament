static uint3 gl_GlobalInvocationID = uint3(0u, 0u, 0u);
RWByteAddressBuffer resultMatrix : register(u2);

float binaryOperation_f1_f1_(inout float a, inout float b) {
  float x_26 = 0.0f;
  if ((b == 0.0f)) {
    return 1.0f;
  }
  float x_21 = b;
  if (!((round((x_21 - (2.0f * floor((x_21 / 2.0f))))) == 1.0f))) {
    x_26 = pow(abs(a), b);
  } else {
    x_26 = (float(sign(a)) * pow(abs(a), b));
  }
  float x_41 = x_26;
  return x_41;
}

void main_1() {
  uint tint_symbol_3 = 0u;
  resultMatrix.GetDimensions(tint_symbol_3);
  uint tint_symbol_4 = ((tint_symbol_3 - 0u) / 4u);
  int index = 0;
  int a_1 = 0;
  float param = 0.0f;
  float param_1 = 0.0f;
  index = asint(gl_GlobalInvocationID.x);
  a_1 = -10;
  int x_63 = index;
  param = -4.0f;
  param_1 = -3.0f;
  float x_68 = binaryOperation_f1_f1_(param, param_1);
  resultMatrix.Store((4u * min(uint(x_63), (tint_symbol_4 - 1u))), asuint(x_68));
  return;
}

struct tint_symbol_1 {
  uint3 gl_GlobalInvocationID_param : SV_DispatchThreadID;
};

void main_inner(uint3 gl_GlobalInvocationID_param) {
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.gl_GlobalInvocationID_param);
  return;
}
