var<workgroup> sh_atomic_failed: u32;

@group(0) @binding(4)
var<storage, read_write> output: u32;

@compute @workgroup_size(256)
fn main(
    @builtin(global_invocation_id) global_id: vec3<u32>,
    @builtin(local_invocation_id) local_id: vec3<u32>,
) {
    let failed = workgroupUniformLoad(&sh_atomic_failed);
    
    if (local_id.x == 0) {
        output = failed;
    }
}
