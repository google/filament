enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn tanh_5b19af() -> f16 {
  var arg_0 = 1.0h;
  var res : f16 = tanh(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = tanh_5b19af();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = tanh_5b19af();
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
  out.prevent_dce = tanh_5b19af();
  return out;
}
