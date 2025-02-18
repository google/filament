enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupOr_f915e3() -> vec4<u32> {
  var res : vec4<u32> = subgroupOr(vec4<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupOr_f915e3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupOr_f915e3();
}
