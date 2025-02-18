struct str {
  int arr[4];
};

typedef int func_ret[4];
func_ret func(inout int pointer[4]) {
  return pointer;
}

static str P = (str)0;

[numthreads(1, 1, 1)]
void main() {
  int r[4] = func(P.arr);
  return;
}
