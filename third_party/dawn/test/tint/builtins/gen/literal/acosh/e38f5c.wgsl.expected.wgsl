@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn acosh_e38f5c() -> vec3<f32> {
  var res : vec3<f32> = acosh(vec3<f32>(1.54308068752288818359f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = acosh_e38f5c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = acosh_e38f5c();
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
  out.prevent_dce = acosh_e38f5c();
  return out;
}
