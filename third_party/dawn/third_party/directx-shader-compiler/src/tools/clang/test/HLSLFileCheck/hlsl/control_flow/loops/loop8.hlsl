// RUN: %dxc -E main -Od -T ps_6_0 %s | FileCheck %s

// CHECK: @main

struct S {
  uint flags;
};

#define FLAG_0 0x4
#define FLAG_1 0x20
#define FLAG_2 0x100

Texture2D t0 : register(t0);
Texture2D t1 : register(t1);

bool has_flag(uint flags, uint flag) {
  return (flags & flag) != 0;
}

float foo(S s, bool add_flag) {

  Texture2D textures[15] = {
    t0, t1, t0, t1, t0,
    t1, t0, t1, t0, t1,
    t0, t1, t0, t1, t0,
  };

  if (add_flag) {
    s.flags |= FLAG_2;
  }

  uint low = has_flag(s.flags, FLAG_0) ? 0 : 1; // Should be 0
  uint high = has_flag(s.flags, FLAG_1) ? 10 : 11; // Should be 11

  float ret = 0;
  [unroll]
  for (uint i = low; i*2 <= high; i++) { // Only 0, 1, 2
    ret += textures[i].Load(i).x;
    high += 1;
  }

  return ret;
}

[RootSignature("DescriptorTable(SRV(t0), SRV(t1))")]
float main() : SV_Target {
  S s = (S)0;
  s.flags |= FLAG_0;
  return foo(s, false);
}


