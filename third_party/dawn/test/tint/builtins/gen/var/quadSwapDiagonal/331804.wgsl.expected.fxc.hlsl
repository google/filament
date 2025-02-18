SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float4 quadSwapDiagonal_331804() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapDiagonal_331804()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapDiagonal_331804()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,16-44): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'


tint executable returned error: exit status 1
