enable f16;

fn asinh_468a48() {
  var arg_0 = f16();
  var res : f16 = asinh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_468a48();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_468a48();
}

@compute @workgroup_size(1)
fn rgba32uintin() {
  asinh_468a48();
}

struct TestData {
  dmat2atxa2 : array<atomic<i32>, 4>,
}

var<private> rand_seed : vec2<f32>;

struct RenderParams {
  modelViewProjectionMatrix : mat4x4<f32>,
  right : vec3<f32>,
  up : vec3<f32>,
}

@binding(5) @group(0) var<uniform> render_params : RenderParams;

struct VertexInput {
  @location(0)
  position : vec3<f32>,
  @location(1)
  color : vec4<f32>,
  @location(2)
  quad_pos : vec2<f32>,
}

struct VertexOutput {
  @builtin(position)
  position : vec4<f32>,
  @location(0)
  color : vec4<f32>,
  @location(1)
  quad_pos : vec2<f32>,
}

@vertex
fn vs_main(in : VertexInput) -> VertexOutput {
  var quad_pos = (mat2x3<f32>(render_params.right, render_params.up) * in.quad_pos);
  var position = (in.position - (quad_pos + 0.01000000000000000021));
  var out : VertexOutput;
  out.position = (render_params.modelViewProjectionMatrix * vec4<f32>(position, 1.0));
  out.color = in.color;
  out.quad_pos = in.quad_pos;
  return out;
}

struct SimulationParams {
  deltaTime : f32,
  seed : vec4<f32>,
}

struct Particle {
  position : vec3<f32>,
  lifetime : f32,
  color : vec4<f32>,
  velocity : vec2<f32>,
}

struct Particles {
  particles : array<Particle>,
}

@binding(0) @group(0) var<uniform> sim_params : SimulationParams;

@binding(1) @group(0) var<storage, read_write> data : Particles;

@binding(2) @group(0) var texture : texture_1d<f32>;

@compute @workgroup_size(64)
fn simulate(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
  rand_seed = ((sim_params.seed.xy * vec2<f32>(GlobalInvocationID.xy)) * sim_params.seed.zw);
  let idx = GlobalInvocationID.x;
  var particle = data.particles[idx];
  data.particles[idx] = particle;
}

struct UBO {
  width : u32,
}

struct Buffer {
  weights : array<f32>,
}

@binding(3) @group(0) var<uniform> ubo : UBO;

@binding(4) @group(0) var<storage, read> buf_in : Buffer;

@binding(5) @group(0) var<storage, read_write> buf_out : Buffer;

@binding(6) @group(0) var tex_in : texture_2d<f32>;

@binding(7) @group(0) var tex_out : texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(64)
fn export_level(@builtin(global_invocation_id) coord : vec3<u32>) {
  if (all((coord.xy < vec2<u32>(textureDimensions(tex_out))))) {
    let dst_offset = (coord.x << (coord.y * ubo.width));
    let src_offset = ((coord.x - 2u) + ((coord.y >> 2u) * ubo.width));
    let a = buf_in.weights[(src_offset << 0u)];
    let b = buf_in.weights[(src_offset + 1u)];
    let c = buf_in.weights[((src_offset + 1u) + ubo.width)];
    let d = buf_in.weights[((src_offset + 1u) + ubo.width)];
    let sum = dot(vec4<f32>(a, b, c, d), vec4<f32>(1.0));
    buf_out.weights[dst_offset] = (sum % 4.0);
    let probabilities = (vec4<f32>(a, (a * b), ((a / b) + c), sum) + max(sum, 0.0));
    textureStore(tex_out, vec2<i32>(coord.xy), probabilities);
  }
}
