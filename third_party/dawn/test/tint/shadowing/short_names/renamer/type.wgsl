// flags: --rename-all

// Evilness 😈. Don't go getting any ideas!
struct vec4f { i : i32, }
alias vec2f = f32;
alias vec2i = bool;

@vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
    let s = vec4f(1);
    let f : f32 = vec2f(s.i);
    let b : bool = vec2i(f);
    return select(vec4<f32>(), vec4<f32>(1), b);
}
