void set_matrix_column(inout float2x4 mat, int col, float4 val) {
  switch (col) {
    case 0: mat[0] = val; break;
    case 1: mat[1] = val; break;
  }
}

cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};

[numthreads(1, 1, 1)]
void main() {
  float2x4 m1 = float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  set_matrix_column(m1, min(uniforms[0].x, 1u), (1.0f).xxxx);
  return;
}
