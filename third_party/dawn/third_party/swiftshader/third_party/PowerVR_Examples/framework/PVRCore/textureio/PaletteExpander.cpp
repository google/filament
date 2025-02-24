/*!
\brief Implementation of methods of the PaletteExpander class.
\file PVRCore/textureio/PaletteExpander.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>
#include "PaletteExpander.h"
#include "PVRCore/Errors.h"

namespace pvr {
PaletteExpander::PaletteExpander(const uint8_t* paletteData, uint32_t paletteSize, uint32_t bytesPerEntry)
	: _paletteData(paletteData), _paletteSize(paletteSize), _bytesPerEntry(bytesPerEntry)
{}

void PaletteExpander::getColorFromIndex(uint32_t index, unsigned char* outputData) const
{
	if (!(_paletteData != 0 && _paletteSize != 0 && _bytesPerEntry != 0)) { throw InvalidOperationError("[PaletteExpander::getColorFromIndex]: Palette Expander was invalid."); }
	if (index >= (_paletteSize / _bytesPerEntry)) { throw IndexOutOfRange("[PaletteExpander::getColorFromIndex]", index, _paletteSize / _bytesPerEntry); }
	memcpy(outputData, &(_paletteData[index * _bytesPerEntry]), _bytesPerEntry);
}
} // namespace pvr
//!\endcond
