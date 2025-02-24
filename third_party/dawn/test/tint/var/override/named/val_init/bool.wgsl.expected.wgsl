override o : bool = true;

override j : bool = false;

@compute @workgroup_size(1)
fn main() {
  if ((o && j)) {
    _ = o;
  }
}
