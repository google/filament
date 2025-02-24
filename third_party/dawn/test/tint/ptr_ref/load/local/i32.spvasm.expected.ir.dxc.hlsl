
void main_1() {
  int i = int(0);
  i = int(123);
  int x_12 = (i + int(1));
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

