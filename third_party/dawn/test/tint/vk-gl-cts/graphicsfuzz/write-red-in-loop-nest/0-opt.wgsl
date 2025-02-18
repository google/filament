var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var m43 : mat4x3<f32>;
  var ll1 : i32;
  var rows : i32;
  var ll4 : i32;
  var ll2 : i32;
  var c : i32;
  var tempm43 : mat4x3<f32>;
  var ll3 : i32;
  var d : i32;
  var r : i32;
  var sums : array<f32, 9u>;
  var idx : i32;
  m43 = mat4x3<f32>(vec3<f32>(1.0, 0.0, 0.0), vec3<f32>(0.0, 1.0, 0.0), vec3<f32>(0.0, 0.0, 1.0), vec3<f32>(0.0, 0.0, 0.0));
  ll1 = 0;
  rows = 2;
  loop {
    if (true) {
    } else {
      break;
    }
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    let x_16 : i32 = ll1;
    if ((x_16 >= 5)) {
      break;
    }
    let x_17 : i32 = ll1;
    ll1 = (x_17 + 1);
    ll4 = 10;
    ll2 = 0;
    c = 0;
    loop {
      let x_19 : i32 = c;
      if ((x_19 < 1)) {
      } else {
        break;
      }
      let x_20 : i32 = ll2;
      if ((x_20 >= 0)) {
        break;
      }
      let x_21 : i32 = ll2;
      ll2 = (x_21 + 1);
      let x_92 : mat4x3<f32> = m43;
      tempm43 = x_92;
      ll3 = 0;
      d = 0;
      loop {
        let x_23 : i32 = ll4;
        if ((1 < x_23)) {
        } else {
          break;
        }
        let x_24 : i32 = d;
        let x_25 : i32 = d;
        let x_26 : i32 = d;
        let x_27 : i32 = r;
        let x_28 : i32 = r;
        let x_29 : i32 = r;
        tempm43[select(0, x_26, ((x_24 >= 0) & (x_25 < 4)))][select(0, x_29, ((x_27 >= 0) & (x_28 < 3)))] = 1.0;

        continuing {
          let x_30 : i32 = d;
          d = (x_30 + 1);
        }
      }
      let x_32 : i32 = idx;
      let x_33 : i32 = idx;
      let x_34 : i32 = idx;
      let x_111 : i32 = select(0, x_34, ((x_32 >= 0) & (x_33 < 9)));
      let x_35 : i32 = c;
      let x_113 : f32 = m43[x_35].y;
      let x_115 : f32 = sums[x_111];
      sums[x_111] = (x_115 + x_113);

      continuing {
        let x_36 : i32 = c;
        c = (x_36 + 1);
      }
    }
    let x_38 : i32 = idx;
    idx = (x_38 + 1);
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
