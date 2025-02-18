
static int zero[3] = (int[3])0;
static const int v[3] = {int(1), int(2), int(3)};
static int init[3] = v;
[numthreads(1, 1, 1)]
void main() {
  int v0[3] = zero;
  int v1[3] = init;
}

