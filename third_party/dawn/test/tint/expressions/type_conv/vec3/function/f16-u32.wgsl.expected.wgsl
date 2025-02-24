enable f16;

var<private> t : f16;

fn m() -> vec3<f16> {
  t = 1.0h;
  return vec3<f16>(t);
}

fn f() {
  var v : vec3<u32> = vec3<u32>(m());
}
