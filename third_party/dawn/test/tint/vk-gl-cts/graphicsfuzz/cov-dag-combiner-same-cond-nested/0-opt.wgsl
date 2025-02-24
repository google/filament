struct buf0 {
  one : f32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  var a : i32;
  var b : i32;
  var c : i32;
  var i : i32;
  var v : vec3<f32>;
  let x_42 : f32 = x_6.one;
  f = x_42;
  a = 1;
  b = 0;
  c = 1;
  i = 0;
  loop {
    let x_47 : i32 = i;
    if ((x_47 < 3)) {
    } else {
      break;
    }
    let x_50 : i32 = i;
    let x_51 : f32 = f;
    let x_52 : i32 = i;
    v[x_50] = (x_51 + f32(x_52));

    continuing {
      let x_56 : i32 = i;
      i = (x_56 + 1);
    }
  }
  let x_59 : f32 = x_6.one;
  if ((x_59 == 1.0)) {
    loop {
      x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);

      continuing {
        let x_67 : i32 = c;
        let x_68 : i32 = a;
        let x_69 : i32 = b;
        break if !((x_67 & (x_68 | x_69)) == 0);
      }
    }
    let x_74 : f32 = x_6.one;
    if ((x_74 == 1.0)) {
      x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
    }
  }
  let x_79 : f32 = v.x;
  let x_83 : f32 = v.y;
  let x_87 : f32 = v.z;
  let x_90 : vec3<f32> = vec3<f32>(select(0.0, 1.0, (x_79 == 1.0)), select(1.0, 0.0, (x_83 == 2.0)), select(1.0, 0.0, (x_87 == 3.0)));
  let x_91 : vec4<f32> = x_GLF_color;
  x_GLF_color = vec4<f32>(x_90.x, x_90.y, x_90.z, x_91.w);
  x_GLF_color.w = 1.0;
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
