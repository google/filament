struct S {
  int i;
};


static S V = (S)0;
void main_1() {
  V.i = int(5);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

