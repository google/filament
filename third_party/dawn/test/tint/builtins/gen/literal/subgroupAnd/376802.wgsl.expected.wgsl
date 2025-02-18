enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupAnd_376802() -> vec2<u32> {
  var res : vec2<u32> = subgroupAnd(vec2<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAnd_376802();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAnd_376802();
}
