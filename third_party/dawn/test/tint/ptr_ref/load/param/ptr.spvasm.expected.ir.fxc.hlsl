
int func(int value, inout int pointer) {
  int x_9 = pointer;
  return (value + x_9);
}

void main_1() {
  int i = int(0);
  i = int(123);
  int x_19 = i;
  int x_18 = func(x_19, i);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

