
static int a = int(0);
static float4 b = (0.0f).xxxx;
static float2x2 c = float2x2((0.0f).xx, (0.0f).xx);
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == int(0)) | ((lhs == int(-2147483648)) & (rhs == int(-1))))) ? (int(1)) : (rhs)));
}

void foo() {
  a = tint_div_i32(a, int(2));
  b = mul(float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx), b);
  c = (c * 2.0f);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

