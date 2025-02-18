fn f(a : i32, b : i32, c : i32) -> i32 {
  return ((a * b) + c);
}

@compute @workgroup_size(1)
fn main() {
  _ = (f(1, 2, 3) + (f(4, 5, 6) * f(7, f(8, 9, 10), 11)));
}
