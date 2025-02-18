enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn fract_eb38ce() -> f16 {
  var res : f16 = fract(1.25h);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fract_eb38ce();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = fract_eb38ce();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f16,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = fract_eb38ce();
  return out;
}
