var<private> t : bool;

fn m() -> bool {
  t = true;
  return bool(t);
}

fn f() {
  var v : u32 = u32(m());
}
