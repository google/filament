static int I = 0;

void main_1() {
  int x_11 = (I + 1);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
