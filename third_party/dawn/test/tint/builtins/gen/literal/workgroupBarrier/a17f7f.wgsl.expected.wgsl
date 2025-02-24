fn workgroupBarrier_a17f7f() {
  workgroupBarrier();
}

@compute @workgroup_size(1)
fn compute_main() {
  workgroupBarrier_a17f7f();
}
