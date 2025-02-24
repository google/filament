@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn abs_5ad50a() -> vec3<i32> {
  var res : vec3<i32> = abs(vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = abs_5ad50a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = abs_5ad50a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = abs_5ad50a();
  return out;
}
