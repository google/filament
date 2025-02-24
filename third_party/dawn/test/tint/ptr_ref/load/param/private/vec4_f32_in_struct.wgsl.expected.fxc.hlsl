struct str {
  float4 i;
};

float4 func(inout float4 pointer) {
  return pointer;
}

static str P = (str)0;

[numthreads(1, 1, 1)]
void main() {
  float4 r = func(P.i);
  return;
}
