enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn subgroupShuffle_8c3fd2() -> vec2<f16> {
  var res : vec2<f16> = subgroupShuffle(vec2<f16>(1.0h), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_8c3fd2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_8c3fd2();
}
