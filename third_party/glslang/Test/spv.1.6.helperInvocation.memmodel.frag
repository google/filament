#version 310 es

#pragma use_vulkan_memory_model

#extension GL_EXT_demote_to_helper_invocation : require

precision highp float;

layout (set=0, binding=0) buffer B {
   float o;
};

void main() {
   demote;
   o = gl_HelperInvocation ? 1.0 : 0.0;
}
