struct str {
  int arr[4];
};


typedef int ary_ret[4];
ary_ret func(inout int pointer[4]) {
  int v[4] = pointer;
  return v;
}

[numthreads(1, 1, 1)]
void main() {
  str F = (str)0;
  int r[4] = func(F.arr);
}

