fn get_f32() -> f32 { return 1.0; }

fn f() {
    var v2 : vec2<f32> = vec2<f32>(get_f32());
    var v3 : vec3<f32> = vec3<f32>(get_f32());
    var v4 : vec4<f32> = vec4<f32>(get_f32());
}
