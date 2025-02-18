<dawn>/test/tint/bug/tint/2201.wgsl:9:9 warning: code is unreachable
        let _e16_ = vec2(false, false);
        ^^^^^^^^^

[numthreads(1, 1, 1)]
void main() {
  while (true) {
    if (true) {
      break;
    } else {
      break;
    }
    bool2 _e16_ = (false).xx;
    {
      if (_e16_.x) { break; }
    }
  }
  return;
}
