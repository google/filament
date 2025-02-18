enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn quadSwapX_bc2013() -> vec3<f16> {
  var res : vec3<f16> = quadSwapX(vec3<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_bc2013();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_bc2013();
}
