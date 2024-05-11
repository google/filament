// This file is part of gltfpack; see gltfpack.h for version/license details
#include "gltfpack.h"

#include <stdio.h>

void comma(std::string& s)
{
	char ch = s.empty() ? 0 : s[s.size() - 1];

	if (ch != 0 && ch != '[' && ch != '{')
		s += ",";
}

void append(std::string& s, size_t v)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "%zu", v);
	s += buf;
}

void append(std::string& s, float v)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "%.9g", v);
	s += buf;
}

void append(std::string& s, const char* v)
{
	s += v;
}

void append(std::string& s, const std::string& v)
{
	s += v;
}

void appendJson(std::string& s, const char* data)
{
	enum State
	{
		None,
		Escape,
		Quoted
	} state = None;

	for (const char* it = data; *it; ++it)
	{
		char ch = *it;

		// whitespace outside of quoted strings can be ignored
		if (state != None || !isspace(ch))
			s += ch;

		// the finite automata tracks whether we're inside a quoted string
		switch (state)
		{
		case None:
			state = (ch == '"') ? Quoted : None;
			break;

		case Quoted:
			state = (ch == '"') ? None : (ch == '\\') ? Escape : Quoted;
			break;

		case Escape:
			state = Quoted;
			break;

		default:
			assert(!"Unexpected parsing state");
		}
	}
}
