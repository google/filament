cbuffer cbuffer_b : register(b0) {
  uint4 b[1];
};

bool func_3() {
  {
    for(int i = 0; (i < asint(b[0].x)); i = (i + 1)) {
      {
        for(int j = -1; (j == 1); j = (j + 1)) {
          return false;
        }
      }
    }
  }
  return false;
}

[numthreads(1, 1, 1)]
void main() {
  func_3();
  return;
}
