enable f16;

var<private> t : bool;

fn m() -> bool {
  t = true;
  return bool(t);
}

fn f() {
  var v : f16 = f16(m());
}
