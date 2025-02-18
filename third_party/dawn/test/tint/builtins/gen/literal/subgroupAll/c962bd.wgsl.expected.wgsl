enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupAll_c962bd() -> i32 {
  var res : bool = subgroupAll(true);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAll_c962bd();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAll_c962bd();
}
