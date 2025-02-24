@group(0) @binding(0) var<storage, read_write> out : mat4x4<f32>;

@compute @workgroup_size(1)
fn f() {
  var m = mat4x4<f32>();
  out = mat4x4(m);
}
