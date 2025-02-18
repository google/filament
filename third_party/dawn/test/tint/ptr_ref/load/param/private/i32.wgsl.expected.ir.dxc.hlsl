
static int P = int(0);
int func(inout int pointer) {
  return pointer;
}

[numthreads(1, 1, 1)]
void main() {
  int r = func(P);
}

