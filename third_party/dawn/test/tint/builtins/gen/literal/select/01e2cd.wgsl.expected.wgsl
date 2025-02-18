@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn select_01e2cd() -> vec3<i32> {
  var res : vec3<i32> = select(vec3<i32>(1i), vec3<i32>(1i), vec3<bool>(true));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = select_01e2cd();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_01e2cd();
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
  out.prevent_dce = select_01e2cd();
  return out;
}
