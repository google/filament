enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupShuffle_fb4ab9() -> vec3<f16> {
  var res : vec3<f16> = subgroupShuffle(vec3<f16>(1.0h), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_fb4ab9();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_fb4ab9();
}
