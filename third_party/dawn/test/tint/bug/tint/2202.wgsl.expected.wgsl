<dawn>/test/tint/bug/tint/2202.wgsl:7:9 warning: code is unreachable
        let _e9 = (vec3<i32>().y >= vec3<i32>().y);
        ^^^^^^^

@compute @workgroup_size(1)
fn main() {
  loop {
    loop {
      return;
    }
    let _e9 = (vec3<i32>().y >= vec3<i32>().y);

    continuing {
      break if (_e9 || _e9);
    }
  }
}
