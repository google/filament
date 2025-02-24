enable f16;

var<private> u = vec2<i32>(1i);

fn f() {
  let v : vec2<f16> = vec2<f16>(u);
}
