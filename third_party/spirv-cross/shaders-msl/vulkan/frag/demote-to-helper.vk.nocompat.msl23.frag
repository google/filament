#version 450
#extension GL_EXT_demote_to_helper_invocation : require

void foo()
{
	demote;
}

void bar()
{
	bool helper = helperInvocationEXT();
}

void main()
{
	foo();
	bar();
}
