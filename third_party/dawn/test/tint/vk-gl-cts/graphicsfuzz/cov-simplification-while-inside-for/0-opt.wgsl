struct buf0 {
  one : i32,
}

struct buf1 {
  zero : i32,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(1) var<uniform> x_9 : buf1;

fn main_1() {
  var i : i32;
  var v : vec4<f32>;
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  i = 0;
  loop {
    let x_38 : i32 = i;
    let x_40 : i32 = x_6.one;
    if ((x_38 < x_40)) {
    } else {
      break;
    }
    loop {
      let x_48 : i32 = x_6.one;
      if ((x_48 == 1)) {
        x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
      }

      continuing {
        break if !(false);
      }
    }

    continuing {
      let x_52 : i32 = i;
      i = (x_52 + 1);
    }
  }
  let x_55 : i32 = x_9.zero;
  v.y = f32(x_55);
  let x_59 : f32 = v.y;
  x_GLF_color.y = x_59;
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
