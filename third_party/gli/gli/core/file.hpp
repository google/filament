/// @brief File helper functions
/// @file gli/core/file.hpp

#pragma once

#include <cstdio>

namespace gli{
namespace detail
{
	FILE* open_file(const char *Filename, const char *mode);
}//namespace detail
}//namespace gli

#include "./file.inl"
