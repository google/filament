enable f16;

var<private> t : f16;

fn m() -> f16 {
  t = 1.0h;
  return f16(t);
}

fn f() {
  var v : f32 = f32(m());
}
