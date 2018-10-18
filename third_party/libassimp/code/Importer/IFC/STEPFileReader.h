/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

#ifndef INCLUDED_AI_STEPFILEREADER_H
#define INCLUDED_AI_STEPFILEREADER_H

#include "code/STEPFile.h"

namespace Assimp {
namespace STEP {

// --------------------------------------------------------------------------
/// @brief  Parsing a STEP file is a twofold procedure.
/// 1) read file header and return to caller, who checks if the
///    file is of a supported schema ..
DB* ReadFileHeader(std::shared_ptr<IOStream> stream);

/// 2) read the actual file contents using a user-supplied set of
///    conversion functions to interpret the data.
void ReadFile(DB& db,const EXPRESS::ConversionSchema& scheme, const char* const* types_to_track, size_t len, const char* const* inverse_indices_to_track, size_t len2);

/// @brief  Helper to read a file.
template <size_t N, size_t N2>
inline
void ReadFile(DB& db,const EXPRESS::ConversionSchema& scheme, const char* const (&arr)[N], const char* const (&arr2)[N2]) {
    return ReadFile(db,scheme,arr,N,arr2,N2);
}

} // ! STEP
} // ! Assimp

#endif
