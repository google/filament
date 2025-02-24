struct VertexInput {
  @location(0)
  position : vec4<f32>,
  @location(1)
  normal : vec3<f32>,
  @location(2)
  tangent : vec4<f32>,
  @location(3)
  texcoord : vec2<f32>,
  @location(6)
  joints : vec4<u32>,
  @location(7)
  weights : vec4<f32>,
  @location(8)
  instance0 : vec4<f32>,
  @location(9)
  instance1 : vec4<f32>,
  @location(10)
  instance2 : vec4<f32>,
  @location(11)
  instance3 : vec4<f32>,
  @location(12)
  instanceColor : vec4<f32>,
}

struct VertexOutput {
  @builtin(position)
  position : vec4<f32>,
  @location(0)
  worldPos : vec3<f32>,
  @location(1)
  view : vec3<f32>,
  @location(2)
  texcoord : vec2<f32>,
  @location(3)
  texcoord2 : vec2<f32>,
  @location(4)
  color : vec4<f32>,
  @location(5)
  instanceColor : vec4<f32>,
  @location(6)
  normal : vec3<f32>,
  @location(7)
  tangent : vec3<f32>,
  @location(8)
  bitangent : vec3<f32>,
}

struct Camera {
  projection : mat4x4<f32>,
  inverseProjection : mat4x4<f32>,
  view : mat4x4<f32>,
  position : vec3<f32>,
  time : f32,
  outputSize : vec2<f32>,
  zNear : f32,
  zFar : f32,
}

@binding(0) @group(0) var<uniform> camera : Camera;

fn getInstanceMatrix(input : VertexInput) -> mat4x4<f32> {
  return mat4x4(input.instance0, input.instance1, input.instance2, input.instance3);
}

struct Joints {
  matrices : array<mat4x4<f32>>,
}

@binding(1) @group(0) var<storage, read> joint : Joints;

@binding(2) @group(0) var<storage, read> inverseBind : Joints;

fn getSkinMatrix(input : VertexInput) -> mat4x4<f32> {
  let joint0 = (joint.matrices[input.joints.x] * inverseBind.matrices[input.joints.x]);
  let joint1 = (joint.matrices[input.joints.y] * inverseBind.matrices[input.joints.y]);
  let joint2 = (joint.matrices[input.joints.z] * inverseBind.matrices[input.joints.z]);
  let joint3 = (joint.matrices[input.joints.w] * inverseBind.matrices[input.joints.w]);
  let skinMatrix = ((((joint0 * input.weights.x) + (joint1 * input.weights.y)) + (joint2 * input.weights.z)) + (joint3 * input.weights.w));
  return skinMatrix;
}

@vertex
fn vertexMain(input : VertexInput) -> VertexOutput {
  var output : VertexOutput;
  let modelMatrix = getSkinMatrix(input);
  output.normal = normalize(((modelMatrix * vec4(input.normal, 0.0))).xyz);
  output.tangent = normalize(((modelMatrix * vec4(input.tangent.xyz, 0.0))).xyz);
  output.bitangent = (cross(output.normal, output.tangent) * input.tangent.w);
  output.color = vec4(1.0);
  output.texcoord = input.texcoord;
  output.instanceColor = input.instanceColor;
  let modelPos = (modelMatrix * input.position);
  output.worldPos = modelPos.xyz;
  output.view = (camera.position - modelPos.xyz);
  output.position = ((camera.projection * camera.view) * modelPos);
  return output;
}
