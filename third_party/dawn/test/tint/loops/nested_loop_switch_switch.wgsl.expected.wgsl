@compute @workgroup_size(1)
fn main() {
  var j = 0;
  for(var i = 0; (i < 2); i += 2) {
    switch(i) {
      case 0: {
        switch(j) {
          case 0: {
            continue;
          }
          default: {
          }
        }
      }
      default: {
      }
    }
  }
}
