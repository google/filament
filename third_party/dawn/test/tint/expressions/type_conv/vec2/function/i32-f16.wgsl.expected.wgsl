enable f16;

var<private> t : i32;

fn m() -> vec2<i32> {
  t = 1i;
  return vec2<i32>(t);
}

fn f() {
  var v : vec2<f16> = vec2<f16>(m());
}
