[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct Inner {
  bool b;
  int4 v;
  float3x3 m;
};
struct Outer {
  Inner a[4];
};

groupshared Outer v;

Outer tint_workgroupUniformLoad_v() {
  GroupMemoryBarrierWithGroupSync();
  Outer result = v;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

Outer foo() {
  return tint_workgroupUniformLoad_v();
}
