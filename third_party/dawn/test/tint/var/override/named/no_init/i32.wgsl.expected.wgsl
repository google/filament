override o : i32;

@compute @workgroup_size(1)
fn main() {
  if ((o == 1)) {
    _ = o;
  }
}
