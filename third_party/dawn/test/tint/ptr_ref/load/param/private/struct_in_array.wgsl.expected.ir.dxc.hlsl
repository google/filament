struct str {
  int i;
};


static str P[4] = (str[4])0;
str func(inout str pointer) {
  str v = pointer;
  return v;
}

[numthreads(1, 1, 1)]
void main() {
  str r = func(P[2u]);
}

