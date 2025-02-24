struct str {
  int arr[4];
};


static str P = (str)0;
void func(inout int pointer[4]) {
  int v[4] = (int[4])0;
  pointer = v;
}

[numthreads(1, 1, 1)]
void main() {
  func(P.arr);
}

