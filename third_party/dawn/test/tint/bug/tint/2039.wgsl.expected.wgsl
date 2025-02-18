@compute @workgroup_size(1, 1, 1)
fn main() {
  var out = 0u;
  loop {
    switch(2) {
      case 1: {
        continue;
      }
      default: {
      }
    }
    out += 1u;

    continuing {
      break if true;
    }
  }
}
