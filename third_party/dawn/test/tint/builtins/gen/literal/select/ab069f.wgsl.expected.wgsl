@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn select_ab069f() -> vec4<i32> {
  var res : vec4<i32> = select(vec4<i32>(1i), vec4<i32>(1i), true);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = select_ab069f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_ab069f();
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
  out.prevent_dce = select_ab069f();
  return out;
}
