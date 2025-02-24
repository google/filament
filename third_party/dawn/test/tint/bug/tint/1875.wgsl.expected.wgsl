struct Outputs {
  data : array<u32>,
}

var<private> count : u32 = 0;

@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

fn push_output(value : u32) {
  outputs.data[count] = value;
  count += 1;
}

@compute @workgroup_size(1)
fn main() {
  _ = &(outputs);
  var a : u32 = 0;
  var b : u32 = 10;
  var c : u32 = 4294967294;
  a++;
  b++;
  c++;
  push_output(a);
  push_output(b);
  push_output(c);
}
