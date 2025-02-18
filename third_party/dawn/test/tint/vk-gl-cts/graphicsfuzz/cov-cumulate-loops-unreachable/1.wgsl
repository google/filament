var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var b : i32;
  var i : i32;
  var i_1 : i32;
  var i_2 : i32;
  var indexable : array<i32, 2u>;
  a = 0;
  b = 1;
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  i = 0;
  loop {
    let x_38 : i32 = i;
    if ((x_38 < 10)) {
    } else {
      break;
    }
    let x_41 : i32 = i;
    if ((x_41 > 1)) {
      let x_45 : i32 = a;
      a = (x_45 + 1);
      if (false) {
        i_1 = 0;
        loop {
          let x_53 : i32 = i_1;
          if ((x_53 < 10)) {
          } else {
            break;
          }
          return;
        }
      }
    }

    continuing {
      let x_56 : i32 = i;
      i = (x_56 + 1);
    }
  }
  i_2 = 0;
  loop {
    let x_62 : i32 = i_2;
    if ((x_62 < 10)) {
    } else {
      break;
    }
    let x_65 : i32 = b;
    indexable = array<i32, 2u>(1, 2);
    let x_67 : i32 = indexable[x_65];
    let x_68 : i32 = a;
    a = (x_68 + x_67);

    continuing {
      let x_70 : i32 = i_2;
      i_2 = (x_70 + 1);
    }
  }
  let x_72 : i32 = a;
  if ((x_72 == 28)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
