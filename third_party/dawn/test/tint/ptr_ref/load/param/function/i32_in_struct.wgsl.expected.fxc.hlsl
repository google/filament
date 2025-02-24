struct str {
  int i;
};

int func(inout int pointer) {
  return pointer;
}

[numthreads(1, 1, 1)]
void main() {
  str F = (str)0;
  int r = func(F.i);
  return;
}
