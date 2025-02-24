static int zero[2][3] = (int[2][3])0;
static int init[2][3] = {{1, 2, 3}, {4, 5, 6}};

[numthreads(1, 1, 1)]
void main() {
  int v0[2][3] = zero;
  int v1[2][3] = init;
  return;
}
