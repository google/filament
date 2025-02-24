struct str {
  int arr[4];
};


static str P = (str)0;
typedef int ary_ret[4];
ary_ret func(inout int pointer[4]) {
  int v[4] = pointer;
  return v;
}

[numthreads(1, 1, 1)]
void main() {
  int r[4] = func(P.arr);
}

