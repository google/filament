struct Buf1 {
  result : i32,
}

alias RTArr = array<u32>;

struct Buf0 {
  values : RTArr,
}

@group(0) @binding(1) var<storage, read_write> x_4 : Buf1;

@group(0) @binding(0) var<storage, read_write> x_7 : Buf0;

fn main_1() {
  var i : u32;
  x_4.result = 1;
  i = 0u;
  loop {
    let x_33 : u32 = i;
    if ((x_33 < 512u)) {
    } else {
      break;
    }
    let x_36 : u32 = i;
    let x_39 : u32 = x_7.values[(x_36 * 2u)];
    let x_40 : u32 = i;
    if ((x_39 != x_40)) {
      x_4.result = 0;
    }

    continuing {
      let x_45 : u32 = i;
      i = (x_45 + bitcast<u32>(1));
    }
  }
  return;
}

@compute @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
