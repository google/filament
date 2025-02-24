var<workgroup> wg : array<atomic<u32>, 4>;

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore(&(wg[1]), 1u);
}
