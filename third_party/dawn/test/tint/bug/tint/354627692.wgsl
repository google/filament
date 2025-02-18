@group(0) @binding(0)
var<storage, read_write> buffer : i32;

@compute @workgroup_size(1)
fn main() {
  var i : i32 = buffer;
  loop {
    continuing {
      loop {
        if (i > 5) {
          i = i * 2;
          break;
        } else {
          i = i * 2;
          break;
        }
      }
      break if i > 10;
    }
  }
  buffer = i;
}
