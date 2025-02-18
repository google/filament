enable f16;

var<private> t : f32;

fn m() -> f32 {
  t = 1.0f;
  return f32(t);
}

fn f() {
  var v : f16 = f16(m());
}
