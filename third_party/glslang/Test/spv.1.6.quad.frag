#version 460 core
#extension GL_KHR_shader_subgroup_basic: require
#extension GL_EXT_shader_quad_control: require
#extension GL_KHR_shader_subgroup_vote: require

layout (full_quads) in;
layout (quad_derivatives) in;

flat in int iInput;

out int bOut;


void main(){
    bool bTemp = false;

    // EXT_shader_quad_control required begin
    bTemp  = bTemp || subgroupQuadAll(iInput > 0);                      // GL_KHR_shader_subgroup_vote
    bTemp  = bTemp || subgroupQuadAny(iInput > 0);                      // GL_KHR_shader_subgroup_vote
    bOut = bTemp == true ? 1 : 0;
}