#version 460

#pragma use_vulkan_memory_model

#extension GL_EXT_nontemporal_keyword: require

layout(location=0) in vec2 in_uv;

layout(binding=1, rgba8) uniform nontemporal image2D u_nontempimage;
layout(binding=2, rgba8) uniform image2D u_image;
layout(binding=3) uniform writeonly nontemporal image2D u_nontempnoformatimage;
layout(binding=4) uniform writeonly image2D u_noformatimage;

void function_accepting_nontemporal(nontemporal writeonly image2D image) {

}

void function_not_accepting_nontemporal(writeonly image2D image) {

}

void main() {
    const ivec2 uv = ivec2(in_uv.x, in_uv.y);
    imageStore(u_nontempimage, uv, imageLoad(u_nontempimage, uv));
    imageStore(u_image, uv, imageLoad(u_image, uv));

    function_accepting_nontemporal(u_nontempnoformatimage);
    function_accepting_nontemporal(u_noformatimage);
    function_not_accepting_nontemporal(u_noformatimage);
    // This would fail to compile
    // function_not_accepting_nontemporal(u_nontempnoformatimage); 
}
