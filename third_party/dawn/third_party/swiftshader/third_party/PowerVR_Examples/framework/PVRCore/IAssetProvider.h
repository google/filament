/*!
\brief Contains the IAssetProvider interface, which decouples the Shell file retrieval functions from the Shell class.
\file PVRCore/IAssetProvider.h
\author PowerVR by Imagination, Developer Technology Team.
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include <cstdint>
#include <string>
#include <memory>

namespace pvr {
class Stream;
/// <summary>The IAssetProvider interface marks a class that provides the getAssetStream function to load assets in a
/// platform independent way.</summary>
class IAssetProvider
{
public:
	/// <summary>Create a Stream from the provided filename. The actual source of the stream is abstracted (filesystem,
	/// resources etc.).</summary>
	/// <param name="filename">The name of the asset. May contain a path or not. Platform-specific paths and built-in
	/// resources should be searched.</param>
	/// <param name="logErrorOnNotFound">OPTIONAL. Set this to false to avoid logging an error when the file is not found.</param>
	/// <returns>A pointer to a Stream. NULL if the asset is not found.</returns>
	/// <remarks>If the file is not found, this function will return a NULL function pointer, and log an error. In
	/// cases where file not found is to be expected (for example, testing for different files due to convention), set
	/// logErrorOnNotFound to false to avoid cluttering the Log.</remarks>
	virtual std::unique_ptr<Stream> getAssetStream(const std::string& filename, bool logErrorOnNotFound = true) const = 0;
};
} // namespace pvr
