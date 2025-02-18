int func(inout int pointer) {
  return pointer;
}

static int P = 0;

[numthreads(1, 1, 1)]
void main() {
  int r = func(P);
  return;
}
