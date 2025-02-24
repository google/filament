var<private> t : i32;

fn m() -> i32 {
  t = 1i;
  return i32(t);
}

fn f() {
  var v : u32 = u32(m());
}
