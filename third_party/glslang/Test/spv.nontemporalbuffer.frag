#version 460

#pragma use_vulkan_memory_model

#extension GL_EXT_nontemporal_keyword: require

layout(binding=0) buffer nontemporal NONTEMPORAL_BUFFER {
    int b_i;
    int b_o;
};
layout(binding=1) buffer BUFFER_NONTEMPORAL {
    nontemporal int bntemp_i;
    nontemporal int bntemp_o;
};
layout(binding=2) uniform nontemporal NONTEMPORAL_UNIFORMS {
    ivec2 u_uv;
};
layout(binding=3) buffer nontemporal NONTEMPORAL_ATOMIC {
    int bn_atom;
};
layout(binding=4) buffer ATOMIC_NONTEMPORAL {
    nontemporal int b_natom;
    int b_atom;
};
layout(binding=5, rgba8) uniform readonly image2D u_image;

layout(location=0) out vec4 out_color;

void main() {
    b_o = b_i;
    bntemp_i = bntemp_o;

    atomicAdd(bn_atom, 1);
    atomicAdd(b_natom, 1);
    atomicAdd(b_atom, 1);
}
