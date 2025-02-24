struct main_inputs {
  uint3 gl_GlobalInvocationID_param : SV_DispatchThreadID;
};


static uint3 gl_GlobalInvocationID = (0u).xxx;
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
    float v = float(sign(a));
    x_26 = (v * pow(abs(a), b));
  }
  float x_41 = x_26;
  return x_41;
}

void main_1() {
  int index = int(0);
  int a_1 = int(0);
  float param = 0.0f;
  float param_1 = 0.0f;
  index = asint(gl_GlobalInvocationID.x);
  a_1 = int(-10);
  int x_63 = index;
  param = -4.0f;
  param_1 = -3.0f;
  float x_68 = binaryOperation_f1_f1_(param, param_1);
  uint v_1 = 0u;
  resultMatrix.GetDimensions(v_1);
  uint v_2 = ((v_1 / 4u) - 1u);
  resultMatrix.Store((0u + (min(uint(x_63), v_2) * 4u)), asuint(x_68));
}

void main_inner(uint3 gl_GlobalInvocationID_param) {
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.gl_GlobalInvocationID_param);
}

