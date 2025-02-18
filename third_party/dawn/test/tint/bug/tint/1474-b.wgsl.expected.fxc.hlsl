<dawn>/test/tint/bug/tint/1474-b.wgsl:7:5 warning: code is unreachable
    let non_uniform_cond = non_uniform_value == 0;
    ^^^^^^^^^^^^^^^^^^^^

RWByteAddressBuffer non_uniform_value : register(u0);

[numthreads(1, 1, 1)]
void main() {
  return;
  bool non_uniform_cond = (asint(non_uniform_value.Load(0u)) == 0);
  if (non_uniform_cond) {
    GroupMemoryBarrierWithGroupSync();
  }
  return;
}
