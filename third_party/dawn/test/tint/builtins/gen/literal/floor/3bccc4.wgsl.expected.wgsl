@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn floor_3bccc4() -> vec4<f32> {
  var res : vec4<f32> = floor(vec4<f32>(1.5f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = floor_3bccc4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = floor_3bccc4();
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
  out.prevent_dce = floor_3bccc4();
  return out;
}
