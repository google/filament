#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in  flat uint inputId;

layout(binding = 0, std430) readonly buffer Ssbo { uint val; uint data[]; } ssbo[];

void main() {
  if(ssbo[nonuniformEXT(inputId)].val==2)
    discard;
  if(ssbo[123].data.length()==25)
    discard;
  }
