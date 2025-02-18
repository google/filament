@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn quantizeToF16_12e50e() -> f32 {
  var arg_0 = 1.0f;
  var res : f32 = quantizeToF16(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quantizeToF16_12e50e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quantizeToF16_12e50e();
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
  out.prevent_dce = quantizeToF16_12e50e();
  return out;
}
