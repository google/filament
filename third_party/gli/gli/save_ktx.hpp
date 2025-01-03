/// @brief Include to save KTX textures to files or memory.
/// @file gli/save_ktx.hpp

#pragma once

#include "texture.hpp"

namespace gli
{
	/// Save a texture storage_linear to a KTX file.
	///
	/// @param Texture Source texture to save
	/// @param Path Path for where to save the file. It must include the filaname and filename extension.
	/// This function ignores the filename extension in the path and save to KTX anyway but keep the requested filename extension.
	/// @return Returns false if the function fails to save the file.
	bool save_ktx(texture const & Texture, char const * Path);

	/// Save a texture storage_linear to a KTX file.
	///
	/// @param Texture Source texture to save
	/// @param Path Path for where to save the file. It must include the filaname and filename extension.
	/// This function ignores the filename extension in the path and save to KTX anyway but keep the requested filename extension.
	/// @return Returns false if the function fails to save the file.
	bool save_ktx(texture const & Texture, std::string const & Path);

	/// Save a texture storage_linear to a KTX file.
	///
	/// @param Texture Source texture to save
	/// @param Memory Storage for the KTX container. The function resizes the containers to fit the necessary storage_linear.
	/// @return Returns false if the function fails to save the file.
	bool save_ktx(texture const & Texture, std::vector<char> & Memory);
}//namespace gli

#include "./core/save_ktx.inl"
