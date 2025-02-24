@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn reverseBits_e1f4c1() -> vec2<u32> {
  var res : vec2<u32> = reverseBits(vec2<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = reverseBits_e1f4c1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = reverseBits_e1f4c1();
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
  out.prevent_dce = reverseBits_e1f4c1();
  return out;
}
