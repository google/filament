enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn subgroupMax_a3afe3() -> vec2<f16> {
  var res : vec2<f16> = subgroupMax(vec2<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_a3afe3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_a3afe3();
}
