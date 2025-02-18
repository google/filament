fn storageBarrier_d87211() {
  storageBarrier();
}

@compute @workgroup_size(1)
fn compute_main() {
  storageBarrier_d87211();
}
