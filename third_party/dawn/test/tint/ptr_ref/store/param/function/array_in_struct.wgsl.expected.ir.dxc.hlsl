struct str {
  int arr[4];
};


void func(inout int pointer[4]) {
  int v[4] = (int[4])0;
  pointer = v;
}

[numthreads(1, 1, 1)]
void main() {
  str F = (str)0;
  func(F.arr);
}

