@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn bitcast_c69aaf() -> vec2<i32> {
  var res : vec2<i32> = bitcast<vec2<i32>>(vec2<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_c69aaf();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_c69aaf();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = bitcast_c69aaf();
  return out;
}
