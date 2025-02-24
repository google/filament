enable f16;

var<private> u = vec4<u32>(1u);

fn f() {
  let v : vec4<f16> = vec4<f16>(u);
}
