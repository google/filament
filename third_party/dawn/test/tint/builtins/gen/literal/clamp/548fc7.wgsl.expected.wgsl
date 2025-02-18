@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn clamp_548fc7() -> vec3<u32> {
  var res : vec3<u32> = clamp(vec3<u32>(1u), vec3<u32>(1u), vec3<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = clamp_548fc7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = clamp_548fc7();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = clamp_548fc7();
  return out;
}
