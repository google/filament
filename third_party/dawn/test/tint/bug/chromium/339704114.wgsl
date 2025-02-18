fn a(v : i32) -> i32 { return v; }

@compute @workgroup_size(1)
fn b() {
  _ = false && (0 < a(4294967295));
}
