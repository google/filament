enable f16;

var<private> t : i32;

fn m() -> i32 {
  t = 1i;
  return i32(t);
}

fn f() {
  var v : f16 = f16(m());
}
