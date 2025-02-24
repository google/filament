struct S {
    val: array<vec3f, 3>,
}

fn a() {
    var a = vec4();
    let b = a.x;
    let c = a.zzyy;

    var d = S();
    let e = d.val[2].yzx;
}
