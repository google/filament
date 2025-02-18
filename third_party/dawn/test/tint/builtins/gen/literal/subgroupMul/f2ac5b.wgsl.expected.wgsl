enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupMul_f2ac5b() -> vec4<f16> {
  var res : vec4<f16> = subgroupMul(vec4<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMul_f2ac5b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_f2ac5b();
}
