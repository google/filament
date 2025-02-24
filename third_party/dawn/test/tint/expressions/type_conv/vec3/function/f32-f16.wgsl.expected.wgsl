enable f16;

var<private> t : f32;

fn m() -> vec3<f32> {
  t = 1.0f;
  return vec3<f32>(t);
}

fn f() {
  var v : vec3<f16> = vec3<f16>(m());
}
