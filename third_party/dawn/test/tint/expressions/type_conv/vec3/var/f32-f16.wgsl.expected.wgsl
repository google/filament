enable f16;

var<private> u = vec3<f32>(1.0f);

fn f() {
  let v : vec3<f16> = vec3<f16>(u);
}
