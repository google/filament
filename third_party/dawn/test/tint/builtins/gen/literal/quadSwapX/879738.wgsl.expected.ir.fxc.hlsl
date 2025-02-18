SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 quadSwapX_879738() {
  float2 res = QuadReadAcrossX((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapX_879738()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapX_879738()));
}

FXC validation failure:
<scrubbed_path>(4,16-41): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
