alias Arr = array<u32, 50>;

struct Buf{
  count : u32,
  data : Arr,
};

@group(0) @binding(0) var<storage, read_write> b : Buf;

@compute @workgroup_size(1)
fn main() {
  var i : u32 = 0u;
  loop {
    if (i >= b.count) { break; }
    let p : ptr<storage, u32, read_write> = &b.data[i];
    if ((i % 2u) == 0u) { continue; }
    *p = 0u;              // Set odd elements of the array to 0
    continuing {
      *p = *p * 2u;        // Double the value in the array entry
      i = i + 1u;
    }
  }
  return;
}
