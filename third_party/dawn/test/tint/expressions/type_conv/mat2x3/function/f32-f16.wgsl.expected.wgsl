enable f16;

var<private> t : f32;

fn m() -> mat2x3<f32> {
  t = (t + 1.0f);
  return mat2x3<f32>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
}

fn f() {
  var v : mat2x3<f16> = mat2x3<f16>(m());
}
