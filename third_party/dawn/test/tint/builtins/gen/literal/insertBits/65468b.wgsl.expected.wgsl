@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn insertBits_65468b() -> i32 {
  var res : i32 = insertBits(1i, 1i, 1u, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = insertBits_65468b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = insertBits_65468b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : i32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = insertBits_65468b();
  return out;
}
