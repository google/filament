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

/** @file  ACLoader.h
 *  @brief Declaration of the .ac importer class.
 */
#ifndef AI_AC3DLOADER_H_INCLUDED
#define AI_AC3DLOADER_H_INCLUDED

#include <vector>

#include "BaseImporter.h"
#include <assimp/types.h>

struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiLight;


namespace Assimp    {

// ---------------------------------------------------------------------------
/** AC3D (*.ac) importer class
*/
class AC3DImporter : public BaseImporter
{
public:
    AC3DImporter();
    ~AC3DImporter();



    // Represents an AC3D material
    struct Material
    {
        Material()
            :   rgb     (0.6f,0.6f,0.6f)
            ,   spec    (1.f,1.f,1.f)
            ,   shin    (0.f)
            ,   trans   (0.f)
        {}

        // base color of the material
        aiColor3D rgb;

        // ambient color of the material
        aiColor3D amb;

        // emissive color of the material
        aiColor3D emis;

        // specular color of the material
        aiColor3D spec;

        // shininess exponent
        float shin;

        // transparency. 0 == opaque
        float trans;

        // name of the material. optional.
        std::string name;
    };

    // Represents an AC3D surface
    struct Surface
    {
        Surface()
            :   mat     (0)
            ,   flags   (0)
        {}

        unsigned int mat,flags;

        typedef std::pair<unsigned int, aiVector2D > SurfaceEntry;
        std::vector< SurfaceEntry > entries;
    };

    // Represents an AC3D object
    struct Object
    {
        Object()
            :   type    (World)
            ,   name( "" )
            ,   children()
            ,   texture( "" )
            ,   texRepeat( 1.f, 1.f )
            ,   texOffset( 0.0f, 0.0f )
            ,   rotation()
            ,   translation()
            ,   vertices()
            ,   surfaces()
            ,   numRefs (0)
            ,   subDiv  (0)
            ,   crease()
        {}

        // Type description
        enum Type
        {
            World = 0x0,
            Poly  = 0x1,
            Group = 0x2,
            Light = 0x4
        } type;

        // name of the object
        std::string name;

        // object children
        std::vector<Object> children;

        // texture to be assigned to all surfaces of the object
        std::string texture;

        // texture repat factors (scaling for all coordinates)
        aiVector2D texRepeat, texOffset;

        // rotation matrix
        aiMatrix3x3 rotation;

        // translation vector
        aiVector3D translation;

        // vertices
        std::vector<aiVector3D> vertices;

        // surfaces
        std::vector<Surface> surfaces;

        // number of indices (= num verts in verbose format)
        unsigned int numRefs;

        // number of subdivisions to be performed on the
        // imported data
        unsigned int subDiv;

        // max angle limit for smoothing
        float crease;
    };


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
     * See #BaseImporter::GetInfo for the details */
    const aiImporterDesc* GetInfo () const;

    // -------------------------------------------------------------------
    /** Imports the given file into the given scene structure.
     * See BaseImporter::InternReadFile() for details*/
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

    // -------------------------------------------------------------------
    /** Called prior to ReadFile().
    * The function is a request to the importer to update its configuration
    * basing on the Importer's configuration property list.*/
    void SetupProperties(const Importer* pImp);

private:

    // -------------------------------------------------------------------
    /** Get the next line from the file.
     *  @return false if the end of the file was reached*/
    bool GetNextLine();

    // -------------------------------------------------------------------
    /** Load the object section. This method is called recursively to
     *  load subobjects, the method returns after a 'kids 0' was
     *  encountered.
     *  @objects List of output objects*/
    void LoadObjectSection(std::vector<Object>& objects);

    // -------------------------------------------------------------------
    /** Convert all objects into meshes and nodes.
     *  @param object Current object to work on
     *  @param meshes Pointer to the list of output meshes
     *  @param outMaterials List of output materials
     *  @param materials Material list
     *  @param Scenegraph node for the object */
    aiNode* ConvertObjectSection(Object& object,
        std::vector<aiMesh*>& meshes,
        std::vector<aiMaterial*>& outMaterials,
        const std::vector<Material>& materials,
        aiNode* parent = NULL);

    // -------------------------------------------------------------------
    /** Convert a material
     *  @param object Current object
     *  @param matSrc Source material description
     *  @param matDest Destination material to be filled */
    void ConvertMaterial(const Object& object,
        const Material& matSrc,
        aiMaterial& matDest);

private:


    // points to the next data line
    const char* buffer;

    // Configuration option: if enabled, up to two meshes
    // are generated per material: those faces who have
    // their bf cull flags set are separated.
    bool configSplitBFCull;

    // Configuration switch: subdivision surfaces are only
    // evaluated if the value is true.
    bool configEvalSubdivision;

    // counts how many objects we have in the tree.
    // basing on this information we can find a
    // good estimate how many meshes we'll have in the final scene.
    unsigned int mNumMeshes;

    // current list of light sources
    std::vector<aiLight*>* mLights;

    // name counters
    unsigned int lights, groups, polys, worlds;
};

} // end of namespace Assimp

#endif // AI_AC3DIMPORTER_H_INC
