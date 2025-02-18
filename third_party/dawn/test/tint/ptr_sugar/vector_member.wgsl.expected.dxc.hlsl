void deref() {
  int3 a = int3(0, 0, 0);
  int b = a.x;
  a.x = 42;
}

void no_deref() {
  int3 a = int3(0, 0, 0);
  int b = a.x;
  a.x = 42;
}

[numthreads(1, 1, 1)]
void main() {
  deref();
  no_deref();
  return;
}
