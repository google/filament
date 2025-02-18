enable f16;

var<private> t : f16;

fn m() -> mat4x4<f16> {
  t = (t + 1.0h);
  return mat4x4<f16>(1.0h, 2.0h, 3.0h, 4.0h, 5.0h, 6.0h, 7.0h, 8.0h, 9.0h, 10.0h, 11.0h, 12.0h, 13.0h, 14.0h, 15.0h, 16.0h);
}

fn f() {
  var v : mat4x4<f32> = mat4x4<f32>(m());
}
