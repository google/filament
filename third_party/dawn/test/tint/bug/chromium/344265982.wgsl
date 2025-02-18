@group(0) @binding(0)
var<storage, read_write> buffer: array<i32, 4>;

fn foo(arg: ptr<storage, array<i32, 4>, read_write>) {
  for (var i = 0; i < 4; i++) {
    switch (arg[i]) {
      case 1: {
        continue;
      }
      default: {
        arg[i] = 2;
      }
    }
  }
}

@fragment
fn main() {
  foo(&buffer);
}
