struct S {
    v: vec3<u32>,
};

var<private> P : S;

fn f() {
    P.v = vec3<u32>(1u, 2u, 3u);

    P.v.x = 1u;
    P.v.y = 2u;
    P.v.z = 3u;
}
