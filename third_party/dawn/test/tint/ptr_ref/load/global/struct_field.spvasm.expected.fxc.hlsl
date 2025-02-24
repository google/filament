struct S {
  int i;
};

static S V = (S)0;

void main_1() {
  int i = 0;
  i = V.i;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
