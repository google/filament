@group(0) @binding(2) var<storage, read> _storage : vec2<i32>;

fn main() {
  var vec : vec2<i32> = vec2<i32>(0);
  loop {
    if ((vec.y >= _storage.y)) {
      break;
    }
    if ((vec.y >= 0)) {
      break;
    }
  }
}
