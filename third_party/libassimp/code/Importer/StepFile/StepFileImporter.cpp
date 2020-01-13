/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team



All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

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
---------------------------------------------------------------------------
*/

#ifndef ASSIMP_BUILD_NO_STEP_IMPORTER

#include "StepFileImporter.h"
#include "../../Importer/STEPParser/STEPFileReader.h"
#include <assimp/importerdesc.h>
#include <assimp/DefaultIOSystem.h>

namespace Assimp {
namespace StepFile {

using namespace STEP;

static const aiImporterDesc desc = { "StepFile Importer",
                                "",
                                "",
                                "",
                                0,
                                0,
                                0,
                                0,
                                0,
                                "stp" };

StepFileImporter::StepFileImporter()
: BaseImporter() {

}

StepFileImporter::~StepFileImporter() {

}

bool StepFileImporter::CanRead(const std::string& file, IOSystem* pIOHandler, bool checkSig) const {
    const std::string &extension = GetExtension(file);
    if ( extension == "stp" || extension == "step" ) {
        return true;
    } else if ((!extension.length() || checkSig) && pIOHandler) {
        const char* tokens[] = { "ISO-10303-21" };
        const bool found(SearchFileHeaderForToken(pIOHandler, file, tokens, 1));
        return found;
    }

    return false;
}

const aiImporterDesc *StepFileImporter::GetInfo() const {
    return &desc;
}

static const std::string mode = "rb";
static const std::string StepFileSchema = "CONFIG_CONTROL_DESIGN";

void StepFileImporter::InternReadFile(const std::string &file, aiScene* pScene, IOSystem* pIOHandler) {
    // Read file into memory
    std::shared_ptr<IOStream> fileStream(pIOHandler->Open(file, mode));
    if (!fileStream.get()) {
        throw DeadlyImportError("Failed to open file " + file + ".");
    }

    std::unique_ptr<STEP::DB> db(STEP::ReadFileHeader(fileStream));
    const STEP::HeaderInfo& head = static_cast<const STEP::DB&>(*db).GetHeader();
    if (!head.fileSchema.size() || head.fileSchema != StepFileSchema) {
        DeadlyImportError("Unrecognized file schema: " + head.fileSchema);
    }
}

} // Namespace StepFile
} // Namespace Assimp

#endif // ASSIMP_BUILD_NO_STEP_IMPORTER

