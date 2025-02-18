var<private> t : u32;

fn m() -> u32 {
  t = 1u;
  return u32(t);
}

fn f() {
  var v : bool = bool(m());
}
