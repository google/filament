struct S {
  int i;
};


static S V = (S)0;
void main_1() {
  int i = int(0);
  i = V.i;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

