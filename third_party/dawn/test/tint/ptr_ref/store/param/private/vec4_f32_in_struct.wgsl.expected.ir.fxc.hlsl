struct str {
  float4 i;
};


static str P = (str)0;
void func(inout float4 pointer) {
  pointer = (0.0f).xxxx;
}

[numthreads(1, 1, 1)]
void main() {
  func(P.i);
}

