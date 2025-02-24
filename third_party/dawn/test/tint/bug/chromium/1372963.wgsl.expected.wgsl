fn g() -> vec4<i32> {
  return (vec4(-(0)) << vec4(2147483649));
}

@fragment
fn main() {
  g();
}
