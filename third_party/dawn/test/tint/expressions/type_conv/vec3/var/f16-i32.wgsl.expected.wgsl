enable f16;

var<private> u = vec3<f16>(1.0h);

fn f() {
  let v : vec3<i32> = vec3<i32>(u);
}
