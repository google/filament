var<private> I : i32;

@compute @workgroup_size(1)
fn main() {
  I = 123;
  I = ((100 + 20) + 3);
}
