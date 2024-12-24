/// @brief Include to load KTX textures from files or memory.
/// @file gli/load_ktx.hpp

#pragma once

#include "texture.hpp"

namespace gli
{
	/// Loads a texture storage_linear from KTX file. Returns an empty storage_linear in case of failure.
	///
	/// @param Path Path of the file to open including filaname and filename extension
	texture load_ktx(char const* Path);

	/// Loads a texture storage_linear from KTX file. Returns an empty storage_linear in case of failure.
	///
	/// @param Path Path of the file to open including filaname and filename extension
	texture load_ktx(std::string const& Path);

	/// Loads a texture storage_linear from KTX memory. Returns an empty storage_linear in case of failure.
	///
	/// @param Data Pointer to the beginning of the texture container data to read
	/// @param Size Size of texture container Data to read
	texture load_ktx(char const* Data, std::size_t Size);
}//namespace gli

#include "./core/load_ktx.inl"
