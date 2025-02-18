enable f16;

var<private> u = vec3<u32>(1u);

fn f() {
  let v : vec3<f16> = vec3<f16>(u);
}
