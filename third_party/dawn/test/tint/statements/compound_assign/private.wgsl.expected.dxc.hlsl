[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int a = 0;
static float4 b = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float2x2 c = float2x2(0.0f, 0.0f, 0.0f, 0.0f);

int tint_div(int lhs, int rhs) {
  return (lhs / (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs));
}

void foo() {
  a = tint_div(a, 2);
  b = mul(float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx), b);
  c = (c * 2.0f);
}
