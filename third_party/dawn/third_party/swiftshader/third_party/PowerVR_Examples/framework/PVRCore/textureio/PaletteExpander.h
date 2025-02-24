/*!
\brief Internally used by some texture readers.
\file PVRCore/textureio/PaletteExpander.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include <cstdint>

namespace pvr {
/// <summary>Internally used by some texture readers.</summary>
class PaletteExpander
{
public:
	/// <summary>Constructor. Initialized with new palette data</summary>
	/// <param name="paletteData">The palette data</param>
	/// <param name="paletteSize">The size of the palete data</param>
	/// <param name="bytesPerEntry">The number of bytes each entry takes</param>
	PaletteExpander(const uint8_t* paletteData, uint32_t paletteSize, uint32_t bytesPerEntry);

	/// <summary>Gets a color for the specified index.</summary>
	/// <param name="index">The index of the entry to retrieve. Throws if overruns the palette</param>
	/// <param name="outputData">A pointer to the palette color for the specified index is returned here</param>
	void getColorFromIndex(uint32_t index, unsigned char* outputData) const;

private:
	const uint8_t* _paletteData;
	const uint32_t _paletteSize;
	const uint32_t _bytesPerEntry;

	// Declare this as private to avoid warnings - the compiler can't generate it because of the const members
	const PaletteExpander& operator=(const PaletteExpander&);
};
} // namespace pvr
