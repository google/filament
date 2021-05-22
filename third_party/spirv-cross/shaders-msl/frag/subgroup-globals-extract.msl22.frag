#version 450
#extension GL_KHR_shader_subgroup_basic : require
#extension GL_KHR_shader_subgroup_ballot : require

layout(location = 0) out uvec2 FragColor;

uint sub1() {
	return subgroupBallotFindLSB(uvec4(1,2,3,4));
}

uint sub2() {
	return subgroupBallotFindMSB(uvec4(1,2,3,4));
}

uint sub3() {
	return subgroupBallotBitCount(uvec4(1,2,3,4));
}

uint sub4() {
	return subgroupBallotInclusiveBitCount(uvec4(1,2,3,4));
}

uint sub5() {
	return subgroupBallotExclusiveBitCount(uvec4(1,2,3,4));
}

void main()
{
	FragColor.x = sub1() + sub2() + sub3() + sub4() + sub5();
}
