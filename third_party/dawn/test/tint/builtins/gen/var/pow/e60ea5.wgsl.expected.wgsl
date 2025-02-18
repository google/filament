@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn pow_e60ea5() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.0f);
  var arg_1 = vec2<f32>(1.0f);
  var res : vec2<f32> = pow(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = pow_e60ea5();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = pow_e60ea5();
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
  out.prevent_dce = pow_e60ea5();
  return out;
}
