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

/** @file NFFLoader.h
 *  @brief Declaration of the NFF importer class.
 */
#ifndef AI_NFFLOADER_H_INCLUDED
#define AI_NFFLOADER_H_INCLUDED

#include "BaseImporter.h"
#include <assimp/types.h>
#include <assimp/material.h>
#include <vector>


namespace Assimp    {

// ----------------------------------------------------------------------------------
/** NFF (Neutral File Format) Importer class.
 *
 * The class implements both Eric Haynes NFF format and Sense8's NFF (NFF2) format.
 * Both are quite different and the loading code is somewhat dirty at
 * the moment. Sense8 should be moved to a separate loader.
*/
class NFFImporter : public BaseImporter
{
public:
    NFFImporter();
    ~NFFImporter();


public:

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

private:


    // describes face material properties
    struct ShadingInfo
    {
        ShadingInfo()
            : color     (0.6f,0.6f,0.6f)
            , diffuse   (1.f,1.f,1.f)
            , specular  (1.f,1.f,1.f)
            , ambient   (0.f,0.f,0.f)
            , emissive  (0.f,0.f,0.f)
            , refracti  (1.f)
            , twoSided  (false) // for NFF2
            , shaded    (true)  // for NFF2
            , opacity   (1.f)
            , shininess (0.f)
            , mapping   (aiTextureMapping_UV)
        {}

        aiColor3D color,diffuse,specular,ambient,emissive;
        float refracti;

        std::string texFile;

        // For NFF2
        bool twoSided;
        bool shaded;
        float opacity, shininess;

        std::string name;

        // texture mapping to be generated for the mesh - uv is the default
        // it means: use UV if there, nothing otherwise. This property is
        // used for locked meshes.
        aiTextureMapping mapping;

        // shininess is ignored for the moment
        bool operator == (const ShadingInfo& other) const
        {
            return color == other.color     &&
                diffuse  == other.diffuse   &&
                specular == other.specular  &&
                ambient  == other.ambient   &&
                refracti == other.refracti  &&
                texFile  == other.texFile   &&
                twoSided == other.twoSided  &&
                shaded   == other.shaded;

            // Some properties from NFF2 aren't compared by this operator.
            // Comparing MeshInfo::matIndex should do that.
        }
    };

    // describes a NFF light source
    struct Light
    {
        Light()
            : intensity (1.f)
            , color     (1.f,1.f,1.f)
        {}

        aiVector3D position;
        float intensity;
        aiColor3D color;
    };

    enum PatchType
    {
        PatchType_Simple = 0x0,
        PatchType_Normals = 0x1,
        PatchType_UVAndNormals = 0x2
    };

    // describes a NFF mesh
    struct MeshInfo
    {
        MeshInfo(PatchType _pType, bool bL = false)
            : pType     (_pType)
            , bLocked   (bL)
            , radius    (1.f,1.f,1.f)
            , dir       (0.f,1.f,0.f)
            , matIndex  (0)
        {
            name[0] = '\0'; // by default meshes are unnamed
        }

        ShadingInfo shader;
        PatchType pType;
        bool bLocked;

        // for spheres, cones and cylinders: center point of the object
        aiVector3D center, radius, dir;

        char name[128];

        std::vector<aiVector3D> vertices, normals, uvs;
        std::vector<unsigned int> faces;

        // for NFF2
        std::vector<aiColor4D>  colors;
        unsigned int matIndex;
    };


    // -------------------------------------------------------------------
    /** Loads the material table for the NFF2 file format from an
     *  external file.
     *
     *  @param output Receives the list of output meshes
     *  @param path Path to the file (abs. or rel.)
     *  @param pIOHandler IOSystem to be used to open the file
    */
    void LoadNFF2MaterialTable(std::vector<ShadingInfo>& output,
        const std::string& path, IOSystem* pIOHandler);

};

} // end of namespace Assimp

#endif // AI_NFFIMPORTER_H_IN
