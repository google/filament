#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_image_load_formatted : enable
#extension GL_EXT_ray_query : enable

layout(location = 0) in  flat uint inputId;

layout(binding = 0) uniform sampler2D smp_textures[];
layout(binding = 1) uniform sampler smp[];
layout(binding = 2) uniform texture2D textures[];
layout(binding = 3, std430) readonly buffer Ssbo { uint val; uint data[]; } ssbo[];
layout(binding = 4, std140) uniform Ubo { uint val; } ubo[];
layout(binding = 5) uniform image2D images[];
layout(binding = 6) uniform accelerationStructureEXT tlas[];

void implicit_combined_texture() {
  vec4 d = textureLod(smp_textures[nonuniformEXT(inputId)],vec2(0,0),0);
  if(d.a>0.5)
    discard;
  }

void implicit_texture() {
  vec4 d = textureLod(sampler2D(textures[nonuniformEXT(inputId)], smp[nonuniformEXT(inputId+8)]),vec2(0,0),0);
  if(d.a>0.5)
    discard;
  }

void implicit_ssbo() {
  if(ssbo[nonuniformEXT(inputId)].val==2)
    discard;
  if(ssbo[123].data.length()==25)
    discard;
  }

void implicit_ubo() {
  if(ubo[nonuniformEXT(inputId)].val==2)
    discard;
  }

void implicit_image() {
  vec4 d = imageLoad(images[nonuniformEXT(inputId)],ivec2(0,0));
  if(d.a>0.5)
    discard;
  }

void implicit_tlas() {
  rayQueryEXT rayQuery;
  rayQueryInitializeEXT(rayQuery, tlas[inputId], 0, 0xFF, vec3(0), 0.01, vec3(1), 1);
  rayQueryProceedEXT(rayQuery);
  }

void explicit_comb_texture(in sampler2D tex) {
  vec4 d = textureLod(tex,vec2(0,0),0);
  if(d.a>0.5)
    discard;
  }

void explicit_texture(in texture2D tex, in sampler smp) {
  vec4 d = textureLod(sampler2D(tex,smp),vec2(0,0),0);
  if(d.a>0.5)
    discard;
  }

void explicit_image(in image2D tex) {
  vec4 d = imageLoad(tex,ivec2(0,0));
  if(d.a>0.5)
    discard;
  }

void explicit_tlas(in accelerationStructureEXT tlas) {
  rayQueryEXT rayQuery;
  rayQueryInitializeEXT(rayQuery, tlas, 0, 0xFF, vec3(0), 0.01, vec3(1), 1);
  rayQueryProceedEXT(rayQuery);
  }

void main() {
  implicit_combined_texture();
  implicit_texture();
  implicit_ssbo();
  implicit_ubo();
  implicit_image();
  implicit_tlas();

  explicit_comb_texture(smp_textures[nonuniformEXT(inputId)]);
  explicit_texture(textures[nonuniformEXT(inputId)], smp[nonuniformEXT(inputId)]);
  explicit_image(images[nonuniformEXT(inputId)]);
  explicit_tlas(tlas[inputId]);
  }
