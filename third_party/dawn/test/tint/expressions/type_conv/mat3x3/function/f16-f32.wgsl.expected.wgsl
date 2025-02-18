enable f16;

var<private> t : f16;

fn m() -> mat3x3<f16> {
  t = (t + 1.0h);
  return mat3x3<f16>(1.0h, 2.0h, 3.0h, 4.0h, 5.0h, 6.0h, 7.0h, 8.0h, 9.0h);
}

fn f() {
  var v : mat3x3<f32> = mat3x3<f32>(m());
}
