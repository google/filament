struct S {
  int data[64];
};

groupshared S s;

void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 64u); idx = (idx + 1u)) {
      uint i = idx;
      s.data[i] = 0;
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

cbuffer cbuffer_ubo : register(b0) {
  uint4 ubo[1];
};

RWByteAddressBuffer result : register(u1);

struct tint_symbol_2 {
  uint local_invocation_index : SV_GroupIndex;
};

void f_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  result.Store(0u, asuint(s.data[min(uint(asint(ubo[0].x)), 63u)]));
}

[numthreads(1, 1, 1)]
void f(tint_symbol_2 tint_symbol_1) {
  f_inner(tint_symbol_1.local_invocation_index);
  return;
}
