enable f16;

var<private> u = vec2<f16>(1.0h);

fn f() {
  let v : vec2<i32> = vec2<i32>(u);
}
