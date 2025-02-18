var<private> t : i32;

fn m() -> vec3<i32> {
  t = 1i;
  return vec3<i32>(t);
}

fn f() {
  var v : vec3<f32> = vec3<f32>(m());
}
