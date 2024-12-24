/// @brief Include to save DDS, KTX or KMG textures to files or memory.
/// @file gli/save.hpp

#pragma once

#include "save_dds.hpp"
#include "save_ktx.hpp"

namespace gli
{
	/// Save a texture storage_linear to file.
	///
	/// @param Texture Source texture to save
	/// @param Path Path for where to save the file. It must include the filaname and filename extension.
	/// The function use the filename extension included in the path to figure out the file container to use.
	/// @return Returns false if the function fails to save the file.
	bool save(texture const & Texture, char const * Path);

	/// Save a texture storage_linear to file.
	///
	/// @param Texture Source texture to save
	/// @param Path Path for where to save the file. It must include the filaname and filename extension.
	/// The function use the filename extension included in the path to figure out the file container to use.
	/// @return Returns false if the function fails to save the file.
	bool save(texture const & Texture, std::string const & Path);
}//namespace gli

#include "./core/save.inl"
