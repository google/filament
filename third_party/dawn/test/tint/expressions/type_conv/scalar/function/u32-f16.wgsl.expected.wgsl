enable f16;

var<private> t : u32;

fn m() -> u32 {
  t = 1u;
  return u32(t);
}

fn f() {
  var v : f16 = f16(m());
}
