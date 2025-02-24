enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn quadSwapX_1e1086() -> i32 {
  var res : i32 = quadSwapX(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_1e1086();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_1e1086();
}
