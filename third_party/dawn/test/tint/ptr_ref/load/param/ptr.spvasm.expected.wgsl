fn func(value : i32, pointer : ptr<function, i32>) -> i32 {
  let x_9 = *(pointer);
  return (value + x_9);
}

fn main_1() {
  var i = 0i;
  i = 123i;
  let x_19 = i;
  let x_18 = func(x_19, &(i));
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
