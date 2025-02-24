enable f16;

var<private> u = vec2<f16>(1.0h);

fn f() {
  let v : vec2<bool> = vec2<bool>(u);
}
