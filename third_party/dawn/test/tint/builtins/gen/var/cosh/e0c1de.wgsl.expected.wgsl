@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn cosh_e0c1de() -> vec4<f32> {
  var arg_0 = vec4<f32>(0.0f);
  var res : vec4<f32> = cosh(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = cosh_e0c1de();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = cosh_e0c1de();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = cosh_e0c1de();
  return out;
}
