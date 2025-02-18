fn deref() {
  var a : mat2x3<f32>;
  let p = &a;
  var b = (*p)[0];
  (*p)[0] = vec3<f32>(1.0, 2.0, 3.0);
}

fn no_deref() {
  var a : mat2x3<f32>;
  let p = &a;
  var b = p[0];
  p[0] = vec3<f32>(1.0, 2.0, 3.0);
}

@compute @workgroup_size(1)
fn main() {
  deref();
  no_deref();
}
