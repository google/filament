@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn fma_c10ba3() -> f32 {
  var res : f32 = fma(1.0f, 1.0f, 1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fma_c10ba3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = fma_c10ba3();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = fma_c10ba3();
  return out;
}
