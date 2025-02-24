RWByteAddressBuffer s : register(u0);

float f1(float a[4]) {
  return a[3];
}

float f2(float a[3][4]) {
  return a[2][3];
}

float f3(float a[2][3][4]) {
  return a[1][2][3];
}

[numthreads(1, 1, 1)]
void main() {
  float a1[4] = (float[4])0;
  float a2[3][4] = (float[3][4])0;
  float a3[2][3][4] = (float[2][3][4])0;
  float v1 = f1(a1);
  float v2 = f2(a2);
  float v3 = f3(a3);
  s.Store(0u, asuint(((v1 + v2) + v3)));
  return;
}
