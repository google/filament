enable f16;

var<private> u = f16(1.0h);

fn f() {
  let v : u32 = u32(u);
}
