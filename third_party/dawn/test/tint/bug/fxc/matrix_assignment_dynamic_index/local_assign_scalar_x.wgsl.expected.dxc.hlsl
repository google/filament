void set_matrix_scalar(inout float2x4 mat, int col, int row, float val) {
  switch (col) {
    case 0:
      mat[0] = (row.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : mat[0];
      break;
    case 1:
      mat[1] = (row.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : mat[1];
      break;
  }
}

cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};

[numthreads(1, 1, 1)]
void main() {
  float2x4 m1 = float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  set_matrix_scalar(m1, min(uniforms[0].x, 1u), 0, 1.0f);
  return;
}
