enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupMin_c6da7c() -> vec3<f16> {
  var res : vec3<f16> = subgroupMin(vec3<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_c6da7c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_c6da7c();
}
