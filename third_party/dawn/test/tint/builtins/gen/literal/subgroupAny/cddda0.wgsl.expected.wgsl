enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupAny_cddda0() -> i32 {
  var res : bool = subgroupAny(true);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAny_cddda0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAny_cddda0();
}
