
fn f(){
    const m = mat4x2(0, 0, 0, 0, 4., 0, 0, 0); // abstract matrix
    const v = vec2(0, 1);                      // abstract vector
    var i = 1;                                 // runtime-evaluated index
    var a = m[i];                              // materialize m before index
    var b = v[i];                              // materialize v before index
}
