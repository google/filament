struct buf0 {
  zero : i32,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var highSigned : i32;
  var highUnsigned : u32;
  var i : i32;
  var data : array<i32, 2u>;
  var i_1 : u32;
  var x_78 : bool;
  var x_79_phi : bool;
  highSigned = 1;
  highUnsigned = 2u;
  i = 0;
  loop {
    let x_42 : i32 = i;
    let x_43 : i32 = highSigned;
    let x_46 : i32 = x_8.zero;
    if ((x_42 < (min(10, x_43) + x_46))) {
    } else {
      break;
    }
    let x_50 : i32 = i;
    data[x_50] = 5;

    continuing {
      let x_52 : i32 = i;
      i = (x_52 + 1);
    }
  }
  i_1 = 1u;
  loop {
    let x_58 : u32 = i_1;
    let x_59 : u32 = highUnsigned;
    let x_62 : i32 = x_8.zero;
    if ((x_58 < (min(10u, x_59) + bitcast<u32>(x_62)))) {
    } else {
      break;
    }
    let x_67 : u32 = i_1;
    data[x_67] = 6;

    continuing {
      let x_69 : u32 = i_1;
      i_1 = (x_69 + bitcast<u32>(1));
    }
  }
  let x_72 : i32 = data[0];
  let x_73 : bool = (x_72 == 5);
  x_79_phi = x_73;
  if (x_73) {
    let x_77 : i32 = data[1];
    x_78 = (x_77 == 6);
    x_79_phi = x_78;
  }
  let x_79 : bool = x_79_phi;
  if (x_79) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
