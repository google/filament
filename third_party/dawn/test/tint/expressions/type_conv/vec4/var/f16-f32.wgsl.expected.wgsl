enable f16;

var<private> u = vec4<f16>(1.0h);

fn f() {
  let v : vec4<f32> = vec4<f32>(u);
}
