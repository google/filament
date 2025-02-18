enable f16;

var<private> u = f32(1.0f);

fn f() {
  let v : f16 = f16(u);
}
