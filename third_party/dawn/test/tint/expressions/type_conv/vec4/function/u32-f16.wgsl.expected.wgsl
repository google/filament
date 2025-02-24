enable f16;

var<private> t : u32;

fn m() -> vec4<u32> {
  t = 1u;
  return vec4<u32>(t);
}

fn f() {
  var v : vec4<f16> = vec4<f16>(m());
}
