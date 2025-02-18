enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn mix_c1aec6() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  var arg_1 = vec3<f16>(1.0h);
  var arg_2 = 1.0h;
  var res : vec3<f16> = mix(arg_0, arg_1, arg_2);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = mix_c1aec6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = mix_c1aec6();
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
  out.prevent_dce = mix_c1aec6();
  return out;
}
