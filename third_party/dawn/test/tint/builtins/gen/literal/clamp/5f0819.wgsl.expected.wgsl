@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn clamp_5f0819() -> vec3<i32> {
  var res : vec3<i32> = clamp(vec3<i32>(1i), vec3<i32>(1i), vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = clamp_5f0819();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = clamp_5f0819();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = clamp_5f0819();
  return out;
}
