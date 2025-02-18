@group(0) @binding(0) var<storage, read_write> buffer : vec4i;

fn deref() {
  let p = &buffer;
  buffer = (*p).wzyx;
}

fn no_deref() {
  let p = &buffer;
  buffer = p.wzyx;
}

@compute @workgroup_size(1)
fn main() {
  deref();
  no_deref();
}
