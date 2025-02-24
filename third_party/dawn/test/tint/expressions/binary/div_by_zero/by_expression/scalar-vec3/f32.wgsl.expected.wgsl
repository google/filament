@compute @workgroup_size(1)
fn f() {
  var a = 4.0;
  var b = vec3<f32>(0.0, 2.0, 0.0);
  let r : vec3<f32> = (a / (b + b));
}
