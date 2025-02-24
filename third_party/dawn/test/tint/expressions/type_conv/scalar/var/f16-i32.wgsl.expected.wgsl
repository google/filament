enable f16;

var<private> u = f16(1.0h);

fn f() {
  let v : i32 = i32(u);
}
