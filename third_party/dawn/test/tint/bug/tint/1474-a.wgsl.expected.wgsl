@compute @workgroup_size(1, 1, 1)
fn main() {
  const cond = true;
  while(cond) {
    if (cond) {
      break;
    } else {
      return;
    }
  }
  let x = 5;
}
