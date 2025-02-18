<dawn>/test/tint/bug/tint/2201.wgsl:9:9 warning: code is unreachable
        let _e16_ = vec2(false, false);
        ^^^^^^^^^

@compute @workgroup_size(1)
fn main() {
  loop {
    if (true) {
      break;
    } else {
      break;
    }
    let _e16_ = vec2(false, false);

    continuing {
      break if _e16_.x;
    }
  }
}
