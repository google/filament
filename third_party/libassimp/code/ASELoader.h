/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team

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

/** @file  ASELoader.h
 *  @brief Definition of the .ASE importer class.
 */
#ifndef AI_ASELOADER_H_INCLUDED
#define AI_ASELOADER_H_INCLUDED

#include "BaseImporter.h"
#include <assimp/types.h>
#include "ASEParser.h"

struct aiNode;

namespace Assimp {

#ifndef ASSIMP_BUILD_NO_3DS_IMPORTER

// --------------------------------------------------------------------------------
/** Importer class for the 3DS ASE ASCII format.
 *
 */
class ASEImporter : public BaseImporter {
public:
    ASEImporter();
    ~ASEImporter();

    // -------------------------------------------------------------------
    /** Returns whether the class can handle the format of the given file.
     * See BaseImporter::CanRead() for details.
     */
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool checkSig) const;

protected:

    // -------------------------------------------------------------------
    /** Return importer meta information.
     * See #BaseImporter::GetInfo for the details
     */
    const aiImporterDesc* GetInfo () const;


    // -------------------------------------------------------------------
    /** Imports the given file into the given scene structure.
    * See BaseImporter::InternReadFile() for details
    */
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);


    // -------------------------------------------------------------------
    /** Called prior to ReadFile().
    * The function is a request to the importer to update its configuration
    * basing on the Importer's configuration property list.
    */
    void SetupProperties(const Importer* pImp);


private:

    // -------------------------------------------------------------------
    /** Generate normal vectors basing on smoothing groups
     * (in some cases the normal are already contained in the file)
     * \param mesh Mesh to work on
     * \return false if the normals have been recomputed
     */
    bool GenerateNormals(ASE::Mesh& mesh);


    // -------------------------------------------------------------------
    /** Create valid vertex/normal/UV/color/face lists.
     *  All elements are unique, faces have only one set of indices
     *  after this step occurs.
     * \param mesh Mesh to work on
     */
    void BuildUniqueRepresentation(ASE::Mesh& mesh);


    /** Create one-material-per-mesh meshes ;-)
     * \param mesh Mesh to work with
     *  \param Receives the list of all created meshes
     */
    void ConvertMeshes(ASE::Mesh& mesh, std::vector<aiMesh*>& avOut);


    // -------------------------------------------------------------------
    /** Convert a material to a aiMaterial object
     * \param mat Input material
     */
    void ConvertMaterial(ASE::Material& mat);


    // -------------------------------------------------------------------
    /** Setup the final material indices for each mesh
     */
    void BuildMaterialIndices();


    // -------------------------------------------------------------------
    /** Build the node graph
     */
    void BuildNodes(std::vector<ASE::BaseNode*>& nodes);


    // -------------------------------------------------------------------
    /** Build output cameras
     */
    void BuildCameras();


    // -------------------------------------------------------------------
    /** Build output lights
     */
    void BuildLights();


    // -------------------------------------------------------------------
    /** Build output animations
     */
    void BuildAnimations(const std::vector<ASE::BaseNode*>& nodes);


    // -------------------------------------------------------------------
    /** Add sub nodes to a node
     *  \param pcParent parent node to be filled
     *  \param szName Name of the parent node
     *  \param matrix Current transform
     */
    void AddNodes(const std::vector<ASE::BaseNode*>& nodes,
        aiNode* pcParent,const char* szName);

    void AddNodes(const std::vector<ASE::BaseNode*>& nodes,
        aiNode* pcParent,const char* szName,
        const aiMatrix4x4& matrix);

    void AddMeshes(const ASE::BaseNode* snode,aiNode* node);

    // -------------------------------------------------------------------
    /** Generate a default material and add it to the parser's list
     *  Called if no material has been found in the file (rare for ASE,
     *  but not impossible)
     */
    void GenerateDefaultMaterial();

protected:

    /** Parser instance */
    ASE::Parser* mParser;

    /** Buffer to hold the loaded file */
    char* mBuffer;

    /** Scene to be filled */
    aiScene* pcScene;

    /** Config options: Recompute the normals in every case - WA
        for 3DS Max broken ASE normal export */
    bool configRecomputeNormals;
    bool noSkeletonMesh;
};

#endif // ASSIMP_BUILD_NO_3DS_IMPORTER

} // end of namespace Assimp


#endif // AI_3DSIMPORTER_H_INC
