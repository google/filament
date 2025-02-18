@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn cos_c3b486() -> vec2<f32> {
  var res : vec2<f32> = cos(vec2<f32>(0.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = cos_c3b486();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = cos_c3b486();
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
  out.prevent_dce = cos_c3b486();
  return out;
}
