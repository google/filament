var<workgroup> wg : array<array<array<atomic<u32>, 1>, 2>, 3>;

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore(&(wg[2][1][0]), 1u);
}
