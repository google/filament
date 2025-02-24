@compute @workgroup_size(1)
fn f() {
  var a = vec3<f32>(1.0, 2.0, 3.0);
  var b = 0.0;
  let r : vec3<f32> = (a / b);
}
