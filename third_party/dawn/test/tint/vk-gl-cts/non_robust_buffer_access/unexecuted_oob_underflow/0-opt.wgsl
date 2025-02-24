alias Arr = array<i32, 8u>;

struct In2 {
  data_in2 : Arr,
}

alias Arr_1 = array<i32, 1024u>;

struct Out0 {
  data_out0 : Arr_1,
}

alias Arr_2 = array<i32, 512u>;

alias Arr_3 = array<i32, 512u>;

struct In0 {
  data_in0 : Arr_3,
}

struct In1 {
  data_in1 : Arr_3,
}

var<private> gl_WorkGroupID : vec3<u32>;

@group(0) @binding(2) var<storage, read> x_13 : In2;

@group(0) @binding(3) var<storage, read_write> x_15 : Out0;

@group(0) @binding(0) var<storage, read> x_17 : In0;

@group(0) @binding(1) var<storage, read> x_19 : In1;

fn main_1() {
  var base_index_in : u32;
  var base_index_out : u32;
  var index_in0 : i32;
  var index_in1 : i32;
  var index_out0 : i32;
  var index_out1 : i32;
  var condition_index : i32;
  var i : i32;
  var temp0 : i32;
  var temp1 : i32;
  let x_56 : u32 = gl_WorkGroupID.x;
  base_index_in = (128u * x_56);
  let x_59 : u32 = gl_WorkGroupID.x;
  base_index_out = (256u * x_59);
  index_in0 = 0;
  index_in1 = -128;
  index_out0 = 0;
  index_out1 = -128;
  condition_index = 0;
  i = 0;
  loop {
    let x_65 : i32 = i;
    if ((x_65 < 256)) {
    } else {
      break;
    }
    let x_68 : i32 = condition_index;
    let x_70 : i32 = x_13.data_in2[x_68];
    if ((x_70 == 0)) {
      let x_75 : u32 = base_index_out;
      let x_76 : i32 = index_out0;
      let x_79 : u32 = base_index_in;
      let x_80 : i32 = index_in0;
      let x_84 : i32 = x_17.data_in0[(x_79 + bitcast<u32>(x_80))];
      x_15.data_out0[(x_75 + bitcast<u32>(x_76))] = x_84;
      let x_86 : i32 = index_out0;
      index_out0 = (x_86 + 1);
      let x_88 : i32 = index_in1;
      index_in1 = (x_88 + 1);
    } else {
      let x_90 : u32 = base_index_out;
      let x_91 : i32 = index_out1;
      let x_94 : u32 = base_index_in;
      let x_95 : i32 = index_in1;
      let x_99 : i32 = x_19.data_in1[(x_94 + bitcast<u32>(x_95))];
      x_15.data_out0[(x_90 + bitcast<u32>(x_91))] = x_99;
      let x_101 : i32 = index_out1;
      index_out1 = (x_101 + 1);
      let x_103 : i32 = index_in1;
      index_in1 = (x_103 + 1);
    }
    let x_105 : i32 = condition_index;
    let x_108 : i32 = x_13.data_in2[(x_105 + 1)];
    let x_109 : i32 = condition_index;
    condition_index = (x_109 + x_108);
    let x_111 : i32 = index_in0;
    temp0 = x_111;
    let x_112 : i32 = index_in1;
    index_in0 = x_112;
    let x_113 : i32 = temp0;
    index_in1 = x_113;
    let x_114 : i32 = index_out0;
    temp1 = x_114;
    let x_115 : i32 = index_out1;
    index_out0 = x_115;
    let x_116 : i32 = temp1;
    index_out1 = x_116;

    continuing {
      let x_117 : i32 = i;
      i = (x_117 + 1);
    }
  }
  return;
}

@compute @workgroup_size(4, 1, 1)
fn main(@builtin(workgroup_id) gl_WorkGroupID_param : vec3<u32>) {
  gl_WorkGroupID = gl_WorkGroupID_param;
  main_1();
}
