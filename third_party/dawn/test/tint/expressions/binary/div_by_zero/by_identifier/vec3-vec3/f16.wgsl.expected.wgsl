enable f16;

@compute @workgroup_size(1)
fn f() {
  var a = vec3<f16>(1.0h, 2.0h, 3.0h);
  var b = vec3<f16>(0.0h, 5.0h, 0.0h);
  let r : vec3<f16> = (a / b);
}
