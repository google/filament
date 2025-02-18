enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupAdd_6587ff() -> vec3<u32> {
  var res : vec3<u32> = subgroupAdd(vec3<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_6587ff();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_6587ff();
}
