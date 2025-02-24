enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupShuffle_821df9() -> vec3<f16> {
  var res : vec3<f16> = subgroupShuffle(vec3<f16>(1.0h), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_821df9();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_821df9();
}
