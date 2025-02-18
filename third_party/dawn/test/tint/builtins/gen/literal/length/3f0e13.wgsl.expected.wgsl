enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn length_3f0e13() -> f16 {
  var res : f16 = length(vec2<f16>(0.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = length_3f0e13();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = length_3f0e13();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f16,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = length_3f0e13();
  return out;
}
