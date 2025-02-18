enable f16;

var<private> u = vec3<bool>(true);

fn f() {
  let v : vec3<f16> = vec3<f16>(u);
}
