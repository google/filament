enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn quadSwapY_06a67c() -> vec3<u32> {
  var arg_0 = vec3<u32>(1u);
  var res : vec3<u32> = quadSwapY(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_06a67c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_06a67c();
}
