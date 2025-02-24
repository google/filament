enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupAdd_fbc357() -> vec4<u32> {
  var arg_0 = vec4<u32>(1u);
  var res : vec4<u32> = subgroupAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_fbc357();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_fbc357();
}
