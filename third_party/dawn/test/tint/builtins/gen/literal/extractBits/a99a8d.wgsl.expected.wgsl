@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn extractBits_a99a8d() -> vec2<i32> {
  var res : vec2<i32> = extractBits(vec2<i32>(1i), 1u, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = extractBits_a99a8d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = extractBits_a99a8d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = extractBits_a99a8d();
  return out;
}
