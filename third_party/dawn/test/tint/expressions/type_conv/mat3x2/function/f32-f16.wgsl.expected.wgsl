enable f16;

var<private> t : f32;

fn m() -> mat3x2<f32> {
  t = (t + 1.0f);
  return mat3x2<f32>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
}

fn f() {
  var v : mat3x2<f16> = mat3x2<f16>(m());
}
