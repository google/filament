var<private> I = 0i;

fn main_1() {
  I = 123i;
  I = ((100i + 20i) + 3i);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
