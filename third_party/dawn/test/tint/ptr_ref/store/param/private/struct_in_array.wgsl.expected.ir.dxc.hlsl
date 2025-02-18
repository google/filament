struct str {
  int i;
};


static str P[4] = (str[4])0;
void func(inout str pointer) {
  str v = (str)0;
  pointer = v;
}

[numthreads(1, 1, 1)]
void main() {
  func(P[2u]);
}

