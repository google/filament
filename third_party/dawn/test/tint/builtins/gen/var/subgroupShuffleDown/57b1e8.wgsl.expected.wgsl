enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn subgroupShuffleDown_57b1e8() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  var arg_1 = 1u;
  var res : vec2<f16> = subgroupShuffleDown(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_57b1e8();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_57b1e8();
}
