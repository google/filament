////////////////////////////////////////////////////////////////////////////////
// Utilities
////////////////////////////////////////////////////////////////////////////////
var<private> rand_seed : vec2<f32>;

fn rand() -> f32 {
    rand_seed.x = fract(cos(dot(rand_seed, vec2<f32>(23.14077926, 232.61690225))) * 136.8168);
    rand_seed.y = fract(cos(dot(rand_seed, vec2<f32>(54.47856553, 345.84153136))) * 534.7645);
    return rand_seed.y;
}

////////////////////////////////////////////////////////////////////////////////
// Vertex shader
////////////////////////////////////////////////////////////////////////////////
struct RenderParams {
  modelViewProjectionMatrix : mat4x4<f32>,
  right : vec3<f32>,
  up    : vec3<f32>,
};
@binding(0) @group(0) var<uniform> render_params : RenderParams;

struct VertexInput {
  @location(0) position : vec3<f32>,
  @location(1) color    : vec4<f32>,
  @location(2) quad_pos : vec2<f32>, // -1..+1
};

struct VertexOutput {
  @builtin(position) position : vec4<f32>,
  @location(0)       color    : vec4<f32>,
  @location(1)       quad_pos : vec2<f32>, // -1..+1
};

@vertex
fn vs_main(in : VertexInput) -> VertexOutput {
  var quad_pos = mat2x3<f32>(render_params.right, render_params.up) * in.quad_pos;
  var position = in.position + quad_pos * 0.01;
  var out : VertexOutput;
  out.position = render_params.modelViewProjectionMatrix * vec4<f32>(position, 1.0);
  out.color = in.color;
  out.quad_pos = in.quad_pos;
  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Fragment shader
////////////////////////////////////////////////////////////////////////////////
@fragment
fn fs_main(in : VertexOutput) -> @location(0) vec4<f32> {
  var color = in.color;
  // Apply a circular particle alpha mask
  color.a = color.a * max(1.0 - length(in.quad_pos), 0.0);
  return color;
}

////////////////////////////////////////////////////////////////////////////////
// Simulation Compute shader
////////////////////////////////////////////////////////////////////////////////
struct SimulationParams {
  deltaTime : f32,
  seed : vec4<f32>,
};

struct Particle {
  position : vec3<f32>,
  lifetime : f32,
  color    : vec4<f32>,
  velocity : vec3<f32>,
};

struct Particles {
  particles : array<Particle>,
};

@binding(0) @group(0) var<uniform> sim_params : SimulationParams;
@binding(1) @group(0) var<storage, read_write> data : Particles;
@binding(2) @group(0) var texture : texture_2d<f32>;

@compute @workgroup_size(64)
fn simulate(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
  rand_seed = (sim_params.seed.xy + vec2<f32>(GlobalInvocationID.xy)) * sim_params.seed.zw;

  let idx = GlobalInvocationID.x;
  var particle = data.particles[idx];

  // Apply gravity
  particle.velocity.z = particle.velocity.z - sim_params.deltaTime * 0.5;

  // Basic velocity integration
  particle.position = particle.position + sim_params.deltaTime * particle.velocity;

  // Age each particle. Fade out before vanishing.
  particle.lifetime = particle.lifetime - sim_params.deltaTime;
  particle.color.a = smoothstep(0.0, 0.5, particle.lifetime);

  // If the lifetime has gone negative, then the particle is dead and should be
  // respawned.
  if (particle.lifetime < 0.0) {
    // Use the probability map to find where the particle should be spawned.
    // Starting with the 1x1 mip level.
    var coord = vec2<u32>(0, 0);
    for (var level = textureNumLevels(texture) - 1; level > 0; level = level - 1) {
      // Load the probability value from the mip-level
      // Generate a random number and using the probabilty values, pick the
      // next texel in the next largest mip level:
      //
      // 0.0    probabilites.r    probabilites.g    probabilites.b   1.0
      //  |              |              |              |              |
      //  |   TOP-LEFT   |  TOP-RIGHT   | BOTTOM-LEFT  | BOTTOM_RIGHT |
      //
      let probabilites = textureLoad(texture, coord, level);
      let value = vec4<f32>(rand());
      let mask = (value >= vec4<f32>(0.0, probabilites.xyz)) & (value < probabilites);
      coord = coord * 2;
      coord.x += select(0u, 1u, any(mask.yw)); // x  y
      coord.y += select(0u, 1u, any(mask.zw)); // z  w
    }
    let uv = vec2<f32>(coord) / vec2<f32>(textureDimensions(texture));
    particle.position = vec3<f32>((uv - 0.5) * 3.0 * vec2<f32>(1.0, -1.0), 0.0);
    particle.color = textureLoad(texture, coord, 0);
    particle.velocity.x = (rand() - 0.5) * 0.1;
    particle.velocity.y = (rand() - 0.5) * 0.1;
    particle.velocity.z = rand() * 0.3;
    particle.lifetime = 0.5 + rand() * 2.0;
  }

  // Store the new particle value
  data.particles[idx] = particle;
}

struct UBO {
  width : u32,
};

struct Buffer {
  weights : array<f32>,
};

@binding(3) @group(0) var<uniform> ubo : UBO;
@binding(4) @group(0) var<storage, read> buf_in : Buffer;
@binding(5) @group(0) var<storage, read_write> buf_out : Buffer;
@binding(6) @group(0) var tex_in : texture_2d<f32>;
@binding(7) @group(0) var tex_out : texture_storage_2d<rgba8unorm, write>;

////////////////////////////////////////////////////////////////////////////////
// import_level
//
// Loads the alpha channel from a texel of the source image, and writes it to
// the buf_out.weights.
////////////////////////////////////////////////////////////////////////////////
@compute @workgroup_size(64)
fn import_level(@builtin(global_invocation_id) coord : vec3<u32>) {
  _ = &buf_in;
  let offset = coord.x + coord.y * ubo.width;
  buf_out.weights[offset] = textureLoad(tex_in, vec2<i32>(coord.xy), 0).w;
}

////////////////////////////////////////////////////////////////////////////////
// export_level
//
// Loads 4 f32 weight values from buf_in.weights, and stores summed value into
// buf_out.weights, along with the calculated 'probabilty' vec4 values into the
// mip level of tex_out. See simulate() in particle.wgsl to understand the
// probability logic.
////////////////////////////////////////////////////////////////////////////////
@compute @workgroup_size(64)
fn export_level(@builtin(global_invocation_id) coord : vec3<u32>) {
  if (all(coord.xy < vec2<u32>(textureDimensions(tex_out)))) {
    let dst_offset = coord.x    + coord.y    * ubo.width;
    let src_offset = coord.x*2u + coord.y*2u * ubo.width;

    let a = buf_in.weights[src_offset + 0u];
    let b = buf_in.weights[src_offset + 1u];
    let c = buf_in.weights[src_offset + 0u + ubo.width];
    let d = buf_in.weights[src_offset + 1u + ubo.width];
    let sum = dot(vec4<f32>(a, b, c, d), vec4<f32>(1.0));

    buf_out.weights[dst_offset] = sum / 4.0;

    let probabilities = vec4<f32>(a, a+b, a+b+c, sum) / max(sum, 0.0001);
    textureStore(tex_out, vec2<i32>(coord.xy), probabilities);
  }
}
