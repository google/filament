enable f16;

var<private> t : i32;

fn m() -> vec3<i32> {
  t = 1i;
  return vec3<i32>(t);
}

fn f() {
  var v : vec3<f16> = vec3<f16>(m());
}
