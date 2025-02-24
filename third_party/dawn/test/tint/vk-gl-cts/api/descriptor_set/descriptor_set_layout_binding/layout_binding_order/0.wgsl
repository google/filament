struct result {
  res0 : i32,
  res1 : i32,
  res2 : i32,
}

struct block0 {
  data0 : i32,
}

struct block1 {
  data1 : i32,
}

struct block2 {
  data2 : i32,
}

@group(0) @binding(3) var<storage, read_write> x_4 : result;

@group(0) @binding(0) var<uniform> x_6 : block0;

@group(0) @binding(1) var<uniform> x_8 : block1;

@group(0) @binding(2) var<uniform> x_10 : block2;

fn main_1() {
  let x_25 : i32 = x_6.data0;
  x_4.res0 = x_25;
  let x_28 : i32 = x_8.data1;
  x_4.res1 = x_28;
  let x_31 : i32 = x_10.data2;
  x_4.res2 = x_31;
  return;
}

@compute @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
