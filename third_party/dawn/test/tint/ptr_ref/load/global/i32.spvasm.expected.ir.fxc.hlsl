
static int I = int(0);
void main_1() {
  int x_11 = (I + int(1));
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

