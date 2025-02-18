fn foo() -> i32 {
  return 1;
}

@fragment
fn main() {
  var arr = array<f32, 4>();
  for (let a = &arr[foo()]; ;) {
    let x = *a;
    break;
  }
}
