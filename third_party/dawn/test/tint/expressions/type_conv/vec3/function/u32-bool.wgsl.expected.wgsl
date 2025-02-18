var<private> t : u32;

fn m() -> vec3<u32> {
  t = 1u;
  return vec3<u32>(t);
}

fn f() {
  var v : vec3<bool> = vec3<bool>(m());
}
