@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn select_713567() -> vec4<f32> {
  var res : vec4<f32> = select(vec4<f32>(1.0f), vec4<f32>(1.0f), true);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = select_713567();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_713567();
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
  out.prevent_dce = select_713567();
  return out;
}
