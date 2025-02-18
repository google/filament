struct S {
  int i;
};


void main_1() {
  S V = (S)0;
  V.i = int(5);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

