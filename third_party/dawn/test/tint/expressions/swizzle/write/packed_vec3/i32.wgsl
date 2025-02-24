struct S {
    v: vec3<i32>,
};

@group(0) @binding(0) var<storage, read_write> U : S;

fn f() {
    U.v = vec3<i32>(1, 2, 3);

    U.v.x = 1;
    U.v.y = 2;
    U.v.z = 3;
}
