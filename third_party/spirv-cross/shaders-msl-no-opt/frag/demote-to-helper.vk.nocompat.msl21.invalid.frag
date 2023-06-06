#version 450
#extension GL_EXT_demote_to_helper_invocation : require

void main()
{
	//demote;	// FIXME: Not implemented for MSL
	bool helper = helperInvocationEXT();
}
