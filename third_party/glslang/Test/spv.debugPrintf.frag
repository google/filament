#version 450
#extension GL_EXT_debug_printf : enable

void main()
{
    debugPrintfEXT("ASDF \\ \? \x5C %d %d %d", 1, 2, 3);
}
