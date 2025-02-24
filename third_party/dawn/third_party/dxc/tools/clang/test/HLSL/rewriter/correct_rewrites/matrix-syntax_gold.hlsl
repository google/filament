// Rewrite unchanged result:
const matrix m;
void abs_without_using_result() {
  matrix<float, 4, 4> mymatrix;
  abs(mymatrix);
  matrix<float, 1, 4> mymatrix2;
  abs(mymatrix2);
}


void abs_with_assignment() {
  matrix<float, 4, 4> mymatrix;
  matrix<float, 4, 4> absMatrix;
  absMatrix = abs(mymatrix);
}


matrix<float, 4, 4> abs_for_result(matrix<float, 4, 4> value) {
  return abs(value);
}


void fn_use_matrix(matrix<float, 4, 4> value) {
}


void abs_in_argument() {
  matrix<float, 4, 4> mymatrix;
  fn_use_matrix(abs(mymatrix));
}


void matrix_on_demand() {
  float4x4 thematrix;
  float4x4 anotherMatrix;
  bool2x1 boolMatrix;
}


void abs_on_demand() {
  float1x2 f12;
  float1x2 result = abs(f12);
}


void matrix_out_of_bounds() {
}


void main() {
  matrix<float, 4, 4> mymatrix;
  matrix<float, 4, 4> absMatrix = abs(mymatrix);
  matrix<float, 4, 4> absMatrix2 = abs(absMatrix);
  matrix<float, 2, 4> f24;
  float f;
  float2 f2;
  float3 f3;
  float4 f4;
  float farr2[2];
  f = mymatrix._m00;
  f2 = mymatrix._m00_m11;
  f4 = mymatrix._m00_m11_m00_m11;
  f2 = mymatrix._m00_m01;
  mymatrix._m00 = mymatrix._m01;
  mymatrix._m00_m11_m02_m13 = mymatrix._m10_m21_m10_m21;
  f = mymatrix._11;
  f2 = mymatrix._11_11;
  f4 = mymatrix._11_11_44_44;
  f = mymatrix[0][0];
  f = mymatrix[0][1];
  f4 = mymatrix[0];
}


