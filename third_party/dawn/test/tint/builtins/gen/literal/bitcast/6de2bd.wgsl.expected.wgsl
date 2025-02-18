@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn bitcast_6de2bd() -> vec4<i32> {
  var res : vec4<i32> = bitcast<vec4<i32>>(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_6de2bd();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_6de2bd();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = bitcast_6de2bd();
  return out;
}
