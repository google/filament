fn a() {
}

fn _a() {
}

fn b() {
  a();
}

fn _b() {
  _a();
}

@compute @workgroup_size(1)
fn main() {
  b();
  _b();
}
