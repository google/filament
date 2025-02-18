struct str {
  float4 i;
};


static str P = (str)0;
float4 func(inout float4 pointer) {
  return pointer;
}

[numthreads(1, 1, 1)]
void main() {
  float4 r = func(P.i);
}

