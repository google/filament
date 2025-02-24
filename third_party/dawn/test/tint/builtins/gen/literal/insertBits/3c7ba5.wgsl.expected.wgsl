@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn insertBits_3c7ba5() -> vec2<u32> {
  var res : vec2<u32> = insertBits(vec2<u32>(1u), vec2<u32>(1u), 1u, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = insertBits_3c7ba5();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = insertBits_3c7ba5();
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
  out.prevent_dce = insertBits_3c7ba5();
  return out;
}
