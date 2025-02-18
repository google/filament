@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn cosh_377652() -> vec3<f32> {
  var res : vec3<f32> = cosh(vec3<f32>(0.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = cosh_377652();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = cosh_377652();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = cosh_377652();
  return out;
}
