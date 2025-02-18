SKIP: INVALID

void set_vector_element(inout vector<float16_t, 4> vec, int idx, float16_t val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

static uint3 localId = uint3(0u, 0u, 0u);
static uint localIndex = 0u;
static uint3 globalId = uint3(0u, 0u, 0u);
static uint3 numWorkgroups = uint3(0u, 0u, 0u);
static uint3 workgroupId = uint3(0u, 0u, 0u);

uint globalId2Index() {
  return globalId.x;
}

[numthreads(1, 1, 1)]
void main() {
  vector<float16_t, 4> a = (float16_t(0.0h)).xxxx;
  float16_t b = float16_t(1.0h);
  int tint_symbol_1 = 0;
  set_vector_element(a, tint_symbol_1, (a[tint_symbol_1] + b));
  return;
}
FXC validation failure:
<scrubbed_path>(1,38-46): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(2,3-5): error X3004: undeclared identifier 'vec'


tint executable returned error: exit status 1
