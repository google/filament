
static int I = int(0);
void main_1() {
  I = int(123);
  I = int(123);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

