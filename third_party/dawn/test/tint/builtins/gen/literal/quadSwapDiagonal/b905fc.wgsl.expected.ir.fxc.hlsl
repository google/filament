SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float3 quadSwapDiagonal_b905fc() {
  float3 res = QuadReadAcrossDiagonal((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_b905fc()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_b905fc()));
}

FXC validation failure:
<scrubbed_path>(4,16-49): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'


tint executable returned error: exit status 1
