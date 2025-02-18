@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn abs_7326de() -> vec3<u32> {
  var res : vec3<u32> = abs(vec3<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = abs_7326de();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = abs_7326de();
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
  out.prevent_dce = abs_7326de();
  return out;
}
