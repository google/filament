enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn subgroupMin_d85be6() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = subgroupMin(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_d85be6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_d85be6();
}
