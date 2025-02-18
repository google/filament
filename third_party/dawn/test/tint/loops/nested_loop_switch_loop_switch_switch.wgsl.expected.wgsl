@compute @workgroup_size(1)
fn main() {
  var k = 0;
  for(var i = 0; (i < 2); i += 2) {
    switch(i) {
      case 0: {
        for(var j = 0; (j < 2); j += 2) {
          switch(j) {
            case 0: {
              continue;
            }
            case 1: {
              switch(k) {
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
        continue;
      }
      default: {
      }
    }
  }
}
