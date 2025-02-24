@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn atan_ad96e4() -> vec2<f32> {
  var res : vec2<f32> = atan(vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = atan_ad96e4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atan_ad96e4();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = atan_ad96e4();
  return out;
}
