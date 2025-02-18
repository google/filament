struct buf0 {
  sequence : vec4<i32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : vec4<i32>;
  var i : i32;
  var sum : i32;
  a = vec4<i32>(0, 0, 0, 0);
  i = 0;
  loop {
    let x_40 : i32 = i;
    let x_42 : i32 = x_7.sequence.w;
    if ((x_40 < (x_42 + 1))) {
    } else {
      break;
    }
    let x_46 : i32 = i;
    let x_48 : i32 = x_7.sequence.x;
    let x_49 : i32 = i;
    let x_52 : i32 = x_7.sequence[clamp(x_46, x_48, x_49)];
    if ((x_52 == 1)) {
      let x_57 : i32 = i;
      a[x_57] = 5;
    } else {
      let x_59 : i32 = i;
      let x_60 : i32 = i;
      a[x_59] = x_60;
    }

    continuing {
      let x_62 : i32 = i;
      i = (x_62 + 1);
    }
  }
  let x_65 : i32 = a.x;
  let x_67 : i32 = a.y;
  let x_70 : i32 = a.z;
  let x_73 : i32 = a.w;
  sum = (((x_65 + x_67) + x_70) + x_73);
  let x_75 : i32 = sum;
  if ((x_75 == 10)) {
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
