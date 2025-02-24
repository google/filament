@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn inverseSqrt_84407e() -> f32 {
  var res : f32 = inverseSqrt(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = inverseSqrt_84407e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = inverseSqrt_84407e();
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
  out.prevent_dce = inverseSqrt_84407e();
  return out;
}
