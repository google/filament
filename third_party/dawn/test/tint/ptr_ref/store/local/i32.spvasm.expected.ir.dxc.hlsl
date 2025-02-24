
void main_1() {
  int i = int(0);
  i = int(123);
  i = int(123);
  i = int(123);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

