enable f16;

var<private> t : f16;

fn m() -> vec2<f16> {
  t = 1.0h;
  return vec2<f16>(t);
}

fn f() {
  var v : vec2<i32> = vec2<i32>(m());
}
