enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupShuffleDown_10eb45() -> vec4<f16> {
  var res : vec4<f16> = subgroupShuffleDown(vec4<f16>(1.0h), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_10eb45();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_10eb45();
}
