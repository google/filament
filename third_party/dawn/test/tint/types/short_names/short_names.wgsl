@vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
    return vec4f(vec2f(vec2i()), 0.0, 1.0);
}
