@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn select_a2860e() -> vec4<i32> {
  var res : vec4<i32> = select(vec4<i32>(1i), vec4<i32>(1i), vec4<bool>(true));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = select_a2860e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_a2860e();
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
  out.prevent_dce = select_a2860e();
  return out;
}
