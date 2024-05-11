#version 450
#if defined(GL_ARB_gpu_shader_int64)
#extension GL_ARB_gpu_shader_int64 : require
#elif defined(GL_NV_gpu_shader5)
#extension GL_NV_gpu_shader5 : require
#else
#error No extension available for 64-bit integers.
#endif
#extension GL_EXT_shader_realtime_clock : require
#extension GL_ARB_shader_clock : require

void main()
{
    uvec2 _11 = clockRealtime2x32EXT();
    uint64_t _15 = clockRealtimeEXT();
    uvec2 _18 = clock2x32ARB();
    uint64_t _20 = clockARB();
}

