@group(0) @binding(1) var<storage, read_write> buf : array<u32, 1>;

fn g() -> i32 {
  return 0;
}

fn f() -> i32 {
  loop {
    g();
    break;
  }
  let o = g();
  return 0;
}

@compute @workgroup_size(1)
fn main() {
  loop {
    if ((buf[0] == 0u)) {
      break;
    }
    var s = f();
    buf[0] = 0u;
  }
}
