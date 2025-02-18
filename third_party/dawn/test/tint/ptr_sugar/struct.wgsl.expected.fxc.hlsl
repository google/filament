struct S {
  int x;
};

void deref() {
  S a = (S)0;
  int b = a.x;
  a.x = 42;
}

void no_deref() {
  S a = (S)0;
  int b = a.x;
  a.x = 42;
}

[numthreads(1, 1, 1)]
void main() {
  deref();
  no_deref();
  return;
}
