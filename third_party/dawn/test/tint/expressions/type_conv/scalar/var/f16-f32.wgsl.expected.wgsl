enable f16;

var<private> u = f16(1.0h);

fn f() {
  let v : f32 = f32(u);
}
