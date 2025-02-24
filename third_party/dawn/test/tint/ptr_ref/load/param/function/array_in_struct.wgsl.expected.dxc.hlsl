struct str {
  int arr[4];
};

typedef int func_ret[4];
func_ret func(inout int pointer[4]) {
  return pointer;
}

[numthreads(1, 1, 1)]
void main() {
  str F = (str)0;
  int r[4] = func(F.arr);
  return;
}
