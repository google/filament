static int zero[3] = (int[3])0;
static int init[3] = {1, 2, 3};

[numthreads(1, 1, 1)]
void main() {
  int v0[3] = zero;
  int v1[3] = init;
  return;
}
