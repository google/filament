enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupMax_15ccbf() -> vec4<u32> {
  var res : vec4<u32> = subgroupMax(vec4<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_15ccbf();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_15ccbf();
}
