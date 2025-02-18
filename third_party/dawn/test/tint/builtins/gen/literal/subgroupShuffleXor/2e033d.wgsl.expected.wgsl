enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupShuffleXor_2e033d() -> vec4<f16> {
  var res : vec4<f16> = subgroupShuffleXor(vec4<f16>(1.0h), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_2e033d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_2e033d();
}
