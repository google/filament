struct S {
    v: vec3<f32>,
};

@group(0) @binding(0) var<storage, read_write> U : S;

fn f() {
    U.v = vec3<f32>(1.0, 2.0, 3.0);

    U.v.x = 1.0;
    U.v.y = 2.0;
    U.v.z = 3.0;
}
