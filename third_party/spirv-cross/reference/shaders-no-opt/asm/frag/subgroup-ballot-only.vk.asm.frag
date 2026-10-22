#version 450

#if defined(GL_KHR_shader_subgroup_ballot)
#extension GL_KHR_shader_subgroup_ballot : require
#elif defined(GL_NV_shader_thread_group)
#extension GL_NV_shader_thread_group : require
#elif defined(GL_ARB_shader_ballot) && defined(GL_ARB_shader_int64)
#extension GL_ARB_shader_int64 : enable
#extension GL_ARB_shader_ballot : require
#else
#error No extensions available to emulate requested subgroup feature.
#endif

layout(location = 0) flat in uint INDEX;
layout(location = 0) out uvec4 SV_Target;

#if defined(GL_KHR_shader_subgroup_ballot)
#elif defined(GL_NV_shader_thread_group)
uvec4 subgroupBallot(bool v) { return uvec4(ballotThreadNV(v), 0u, 0u, 0u); }
#elif defined(GL_ARB_shader_ballot)
uvec4 subgroupBallot(bool v) { return uvec4(unpackUint2x32(ballotARB(v)), 0u, 0u); }
#endif

void main()
{
    uvec4 _15 = subgroupBallot(INDEX < 100u);
    SV_Target.x = _15.x;
    SV_Target.y = _15.y;
    SV_Target.z = _15.z;
    SV_Target.w = _15.w;
}

