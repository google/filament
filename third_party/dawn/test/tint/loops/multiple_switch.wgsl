@compute @workgroup_size(1)
fn main() {
  var i = 0;
  for (var i = 0; i < 2; i += 1) {
    switch(i) {
      case 0: {
        continue;
      }
      default: {
      }
    }

    switch(i) {
      case 0: {
        continue;
      }
      default: {
      }
    }
  }
}
