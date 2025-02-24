enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupAnd_d2c9a6() -> vec4<u32> {
  var arg_0 = vec4<u32>(1u);
  var res : vec4<u32> = subgroupAnd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAnd_d2c9a6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAnd_d2c9a6();
}
