override o : bool;

override j : bool;

@compute @workgroup_size(1)
fn main() {
  if ((o && j)) {
    _ = 1;
  }
}
