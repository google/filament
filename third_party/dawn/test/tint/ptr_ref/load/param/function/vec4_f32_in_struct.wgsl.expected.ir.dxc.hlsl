struct str {
  float4 i;
};


float4 func(inout float4 pointer) {
  return pointer;
}

[numthreads(1, 1, 1)]
void main() {
  str F = (str)0;
  float4 r = func(F.i);
}

