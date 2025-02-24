enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn quadSwapY_a50fcb() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = quadSwapY(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_a50fcb();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_a50fcb();
}
