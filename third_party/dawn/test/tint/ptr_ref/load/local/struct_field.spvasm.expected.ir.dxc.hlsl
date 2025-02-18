struct S {
  int i;
};


void main_1() {
  int i = int(0);
  S V = (S)0;
  i = V.i;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

