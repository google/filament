// RUN: %dxc %s -T ps_6_0 -Od | FileCheck %s

// When switch's cases have more control flow than just one simple block,
// DVC couldn't deduce the outflowing values.

// CHECK: @main
// CHECK-NOT: switch

uint get(uint p, uint off) {
  if (p & 1) {
    return p << 2;
  }
  else {
    return p + off;
  }
}

uint get_bound(uint param) {
  switch (param) {
    case 0:  return get(param, 1);
    case 1:  return get(param, 2);
    case 2:  return get(param, 3);
    case 3:  return get(param, 4);
    case 4:  return get(param, 5);
    case 10: return get(param, 6);
  }
  return 0;
}

Texture1D<float> t0 : register(t0);

[RootSignature("DescriptorTable(SRV(t0))")]
float main() : SV_Target {
  uint param = 10;
  uint bound = get_bound(param);

  float ret = 0;
  [unroll]
  for (uint i = 0; i < bound; i++) {
    ret += t0[i];
  }

  return ret;
}