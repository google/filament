
uint get_u32() {
  return 1u;
}

void f() {
  uint2 v2 = uint2((get_u32()).xx);
  uint3 v3 = uint3((get_u32()).xxx);
  uint4 v4 = uint4((get_u32()).xxxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

