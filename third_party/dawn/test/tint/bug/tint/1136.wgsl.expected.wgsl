struct Buffer {
  data : u32,
}

@group(0) @binding(0) var<storage, read_write> buffer : Buffer;

fn main() {
  buffer.data = (buffer.data + 1u);
}
