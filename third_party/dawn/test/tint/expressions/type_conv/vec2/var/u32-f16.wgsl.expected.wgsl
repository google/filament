enable f16;

var<private> u = vec2<u32>(1u);

fn f() {
  let v : vec2<f16> = vec2<f16>(u);
}
