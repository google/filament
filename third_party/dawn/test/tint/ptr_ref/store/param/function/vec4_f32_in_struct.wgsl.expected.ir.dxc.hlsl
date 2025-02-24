struct str {
  float4 i;
};


void func(inout float4 pointer) {
  pointer = (0.0f).xxxx;
}

[numthreads(1, 1, 1)]
void main() {
  str F = (str)0;
  func(F.i);
}

