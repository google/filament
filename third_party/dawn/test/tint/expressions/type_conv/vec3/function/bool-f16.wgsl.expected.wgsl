enable f16;

var<private> t : bool;

fn m() -> vec3<bool> {
  t = true;
  return vec3<bool>(t);
}

fn f() {
  var v : vec3<f16> = vec3<f16>(m());
}
