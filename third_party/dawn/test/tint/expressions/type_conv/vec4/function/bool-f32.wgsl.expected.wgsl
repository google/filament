var<private> t : bool;

fn m() -> vec4<bool> {
  t = true;
  return vec4<bool>(t);
}

fn f() {
  var v : vec4<f32> = vec4<f32>(m());
}
