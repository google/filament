@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn bitcast_9bcf71() -> vec2<f32> {
  var res : vec2<f32> = bitcast<vec2<f32>>(vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_9bcf71();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_9bcf71();
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
  out.prevent_dce = bitcast_9bcf71();
  return out;
}
