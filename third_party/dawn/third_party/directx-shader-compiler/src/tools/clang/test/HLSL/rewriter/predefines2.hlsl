#define X 1
#define Y(A, B) ((A) + (B))
float x = X;
float test(float a, float b) {
  return Y(a, b);
}