#version 450

#extension GL_EXT_shader_realtime_clock : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

#extension GL_ARB_shader_clock : enable
#extension GL_ARB_gpu_shader_int64 : enable

void main() {
    // GL_EXT_shader_explicit_arithmetic_types_int64
    uvec2 a = clockRealtime2x32EXT();
    uint64_t b = clockRealtimeEXT();

    // GL_ARB_shader_clock
    uvec2 c = clock2x32ARB();
    uint64_t d = clockARB();
}
