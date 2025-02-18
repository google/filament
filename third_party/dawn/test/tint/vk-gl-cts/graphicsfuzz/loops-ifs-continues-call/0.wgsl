struct BinarySearchObject {
  prime_numbers : array<i32, 10u>,
}

struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn binarySearch_struct_BinarySearchObject_i1_10_1_(obj : ptr<function, BinarySearchObject>) -> i32 {
  var m : i32;
  loop {
    let x_91 : f32 = x_8.injectionSwitch.x;
    if ((x_91 > 1.0)) {
    } else {
      break;
    }
    let x_95 : f32 = x_8.injectionSwitch.x;
    m = i32(x_95);
    let x_14 : i32 = m;
    let x_15 : i32 = (*(obj)).prime_numbers[x_14];
    if ((x_15 == 1)) {
      return 1;
    }
  }
  return 1;
}

fn main_1() {
  var i : i32;
  var obj_1 : BinarySearchObject;
  var param : BinarySearchObject;
  i = 0;
  loop {
    let x_16 : i32 = i;
    if ((x_16 < 10)) {
    } else {
      break;
    }
    let x_17 : i32 = i;
    if ((x_17 != 3)) {
      let x_18 : i32 = i;
      let x_67 : f32 = x_8.injectionSwitch.x;
      if (((x_18 - i32(x_67)) == 4)) {
        let x_21 : i32 = i;
        obj_1.prime_numbers[x_21] = 11;
      } else {
        let x_22 : i32 = i;
        if ((x_22 == 6)) {
          let x_23 : i32 = i;
          obj_1.prime_numbers[x_23] = 17;
        }
        continue;
      }
    }
    loop {

      continuing {
        let x_82 : f32 = x_8.injectionSwitch.y;
        break if !(0.0 > x_82);
      }
    }

    continuing {
      let x_24 : i32 = i;
      i = (x_24 + 1);
    }
  }
  let x_84 : BinarySearchObject = obj_1;
  param = x_84;
  let x_26 : i32 = binarySearch_struct_BinarySearchObject_i1_10_1_(&(param));
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
