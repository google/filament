var<private> t : i32;

fn m() -> vec4<i32> {
  t = 1i;
  return vec4<i32>(t);
}

fn f() {
  var v : vec4<f32> = vec4<f32>(m());
}
