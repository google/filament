@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn bitcast_2b2738() -> vec2<u32> {
  var res : vec2<u32> = bitcast<vec2<u32>>(vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_2b2738();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_2b2738();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = bitcast_2b2738();
  return out;
}
