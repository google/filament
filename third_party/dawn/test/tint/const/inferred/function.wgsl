alias MyArray = array<f32, 10>;

// Function-scope consts
fn const_decls() {
    const v1 = 1;
    const v2 = 1u;
    const v3 = 1.0;

    const v4 = vec3<i32>(1, 1, 1);
    const v5 = vec3<u32>(1u, 1u, 1u);
    const v6 = vec3<f32>(1.0, 1.0, 1.0);

    const v7 = mat3x3<f32>(v6, v6, v6);

    const v8 = MyArray();
}

@fragment
fn main() -> @location(0) vec4<f32> {
    return vec4<f32>(0.0,0.0,0.0,0.0);
}
