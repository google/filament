@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn select_c4a4ef() -> vec4<u32> {
  var res : vec4<u32> = select(vec4<u32>(1u), vec4<u32>(1u), vec4<bool>(true));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = select_c4a4ef();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_c4a4ef();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = select_c4a4ef();
  return out;
}
