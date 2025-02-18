enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn determinant_32bfde() -> f16 {
  var arg_0 = mat4x4<f16>(1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h);
  var res : f16 = determinant(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = determinant_32bfde();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = determinant_32bfde();
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
  out.prevent_dce = determinant_32bfde();
  return out;
}
