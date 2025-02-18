enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn radians_7ea4c7() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  var res : vec3<f16> = radians(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = radians_7ea4c7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = radians_7ea4c7();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<f16>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = radians_7ea4c7();
  return out;
}
