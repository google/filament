@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn dot4I8Packed_881e62() -> i32 {
  var res : i32 = dot4I8Packed(1u, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dot4I8Packed_881e62();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = dot4I8Packed_881e62();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : i32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = dot4I8Packed_881e62();
  return out;
}
