enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupExclusiveMul_000b92() -> vec4<u32> {
  var res : vec4<u32> = subgroupExclusiveMul(vec4<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_000b92();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_000b92();
}
