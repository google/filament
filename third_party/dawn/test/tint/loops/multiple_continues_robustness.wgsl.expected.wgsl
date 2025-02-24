@compute @workgroup_size(1)
fn main() {
  for(var i = 0; (i < 2); i += 1) {
    switch(i) {
      case 0: {
        continue;
      }
      case 1: {
        continue;
      }
      case 2: {
        continue;
      }
      default: {
      }
    }
  }
}
