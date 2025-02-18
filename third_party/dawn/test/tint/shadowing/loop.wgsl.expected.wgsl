@group(0) @binding(0) var<storage, read_write> output : array<i32, 10>;

@compute @workgroup_size(1)
fn foo() {
  var i = 0;
  loop {
    var x = output[i];

    continuing {
      var x = output[x];
      i += x;
      break if (i > 10);
    }
  }
  output[0] = i;
}
