@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn atan2_96057c() -> f32 {
  var arg_0 = 1.0f;
  var arg_1 = 1.0f;
  var res : f32 = atan2(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = atan2_96057c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atan2_96057c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = atan2_96057c();
  return out;
}
