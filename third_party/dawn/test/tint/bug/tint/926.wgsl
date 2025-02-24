 struct DrawIndirectArgs {
  vertexCount : atomic<u32>,
};
@group(0) @binding(5) var<storage, read_write> drawOut : DrawIndirectArgs;

var<private> cubeVerts : u32 = 0u;

@compute @workgroup_size(1)
fn computeMain(@builtin(global_invocation_id) global_id : vec3<u32>) {
  // Increment cubeVerts based on some criteria...

  // This fails SPIR-V validation
  let firstVertex : u32 = atomicAdd(&drawOut.vertexCount, cubeVerts);
}
