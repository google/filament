enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn quadSwapX_19f8ce() -> vec2<u32> {
  var res : vec2<u32> = quadSwapX(vec2<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_19f8ce();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_19f8ce();
}
