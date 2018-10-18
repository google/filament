/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2012, assimp team
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

/** @file  C4DImporter.h
 *  @brief Declaration of the Cinema4D (*.c4d) importer class.
 */
#ifndef INCLUDED_AI_CINEMA_4D_LOADER_H
#define INCLUDED_AI_CINEMA_4D_LOADER_H

#include <assimp/BaseImporter.h>
#include <assimp/LogAux.h>

#include <map>
struct aiNode;
struct aiMesh;
struct aiMaterial;

struct aiImporterDesc;

namespace melange {
    class BaseObject; // c4d_file.h
    class PolygonObject;
    class BaseMaterial;
    class BaseShader;
}

namespace Assimp    {

    // TinyFormatter.h
    namespace Formatter {
        template <typename T,typename TR, typename A> class basic_formatter;
        typedef class basic_formatter< char, std::char_traits<char>, std::allocator<char> > format;
    }

// -------------------------------------------------------------------------------------------
/** Importer class to load Cinema4D files using the Melange library to be obtained from
 *  www.plugincafe.com
 *
 *  Note that Melange is not free software. */
// -------------------------------------------------------------------------------------------
class C4DImporter : public BaseImporter, public LogFunctions<C4DImporter>
{
public:

    C4DImporter();
    ~C4DImporter();


public:

    // --------------------
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool checkSig) const;

protected:

    // --------------------
    const aiImporterDesc* GetInfo () const;

    // --------------------
    void SetupProperties(const Importer* pImp);

    // --------------------
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

private:

    void ReadMaterials(melange::BaseMaterial* mat);
    void RecurseHierarchy(melange::BaseObject* object, aiNode* parent);
    aiMesh* ReadMesh(melange::BaseObject* object);
    unsigned int ResolveMaterial(melange::PolygonObject* obj);

    bool ReadShader(aiMaterial* out, melange::BaseShader* shader);

    std::vector<aiMesh*> meshes;
    std::vector<aiMaterial*> materials;

    typedef std::map<melange::BaseMaterial*, unsigned int> MaterialMap;
    MaterialMap material_mapping;

}; // !class C4DImporter

} // end of namespace Assimp
#endif // INCLUDED_AI_CINEMA_4D_LOADER_H

