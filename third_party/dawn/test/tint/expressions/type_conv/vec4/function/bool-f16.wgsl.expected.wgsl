enable f16;

var<private> t : bool;

fn m() -> vec4<bool> {
  t = true;
  return vec4<bool>(t);
}

fn f() {
  var v : vec4<f16> = vec4<f16>(m());
}
