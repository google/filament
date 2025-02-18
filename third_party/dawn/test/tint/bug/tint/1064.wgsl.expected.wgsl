@fragment
fn main() {
  loop {
    if (false) {
    } else {
      break;
    }

    continuing {
      break if !(true);
    }
  }
}
