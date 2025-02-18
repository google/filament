
int func(inout int pointer) {
  return pointer;
}

[numthreads(1, 1, 1)]
void main() {
  int F = int(0);
  int r = func(F);
}

