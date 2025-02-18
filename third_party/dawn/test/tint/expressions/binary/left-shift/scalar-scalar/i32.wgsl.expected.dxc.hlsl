[numthreads(1, 1, 1)]
void f() {
  int a = 1;
  uint b = 2u;
  int r = (a << (b & 31u));
  return;
}
