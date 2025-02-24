struct MyStruct {
  f1 : f32,
}

alias MyArray = array<f32, 10>;

var<private> v1 = 1;

var<private> v2 = 1u;

var<private> v3 = 1.0;

var<private> v4 = vec3<i32>(1, 1, 1);

var<private> v5 = vec3<u32>(1u, 2u, 3u);

var<private> v6 = vec3<f32>(1.0, 2.0, 3.0);

var<private> v7 = MyStruct(1.0);

var<private> v8 = MyArray();

var<private> v9 = i32();

var<private> v10 = u32();

var<private> v11 = f32();

var<private> v12 = MyStruct();

var<private> v13 = MyStruct();

var<private> v14 = MyArray();

var<private> v15 = vec3(1, 2, 3);

var<private> v16 = vec3(1.0, 2.0, 3.0);

@compute @workgroup_size(1)
fn f() {
  let l1 = v1;
  let l2 = v2;
  let l3 = v3;
  let l4 = v4;
  let l5 = v5;
  let l6 = v6;
  let l7 = v7;
  let l8 = v8;
  let l9 = v9;
  let l10 = v10;
  let l11 = v11;
  let l12 = v12;
  let l13 = v13;
  let l14 = v14;
  let l15 = v15;
  let l16 = v16;
}
