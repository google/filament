
static int zero[2][3] = (int[2][3])0;
static const int v[2][3] = {{int(1), int(2), int(3)}, {int(4), int(5), int(6)}};
static int init[2][3] = v;
[numthreads(1, 1, 1)]
void main() {
  int v0[2][3] = zero;
  int v1[2][3] = init;
}

