struct S {
  int i;
};

void main_1() {
  S V = (S)0;
  V.i = 5;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
