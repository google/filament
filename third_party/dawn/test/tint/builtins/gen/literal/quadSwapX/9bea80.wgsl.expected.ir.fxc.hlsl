SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float quadSwapX_9bea80() {
  float res = QuadReadAcrossX(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_9bea80()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_9bea80()));
}

FXC validation failure:
<scrubbed_path>(4,15-35): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
