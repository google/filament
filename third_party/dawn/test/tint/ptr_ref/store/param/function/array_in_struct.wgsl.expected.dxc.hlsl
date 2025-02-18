struct str {
  int arr[4];
};

void func(inout int pointer[4]) {
  int tint_symbol[4] = (int[4])0;
  pointer = tint_symbol;
}

[numthreads(1, 1, 1)]
void main() {
  str F = (str)0;
  func(F.arr);
  return;
}
