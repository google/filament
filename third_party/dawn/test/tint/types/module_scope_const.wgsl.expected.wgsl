const const_bool : bool = bool();

const const_i32 : i32 = i32();

const const_u32 : u32 = u32();

const const_f32 : f32 = f32();

const const_v2i32 : vec2<i32> = vec2<i32>();

const const_v3u32 : vec3<u32> = vec3<u32>();

const const_v4f32 : vec4<f32> = vec4<f32>();

const const_m3x4 : mat3x4<f32> = mat3x4<f32>();

const const_arr : array<f32, 4> = array<f32, 4>();

@compute @workgroup_size(1)
fn main() {
}
