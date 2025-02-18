int func(int value, inout int pointer) {
  int x_9 = pointer;
  return (value + x_9);
}

void main_1() {
  int i = 0;
  i = 123;
  int x_19 = i;
  int x_18 = func(x_19, i);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
