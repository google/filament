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

/** @file  LWSLoader.h
 *  @brief Declaration of the LightWave scene importer class.
 */
#ifndef AI_LWSLOADER_H_INCLUDED
#define AI_LWSLOADER_H_INCLUDED

#include "LWOFileData.h"
#include <assimp/SceneCombiner.h>
#include <assimp/BaseImporter.h>

struct aiImporterDesc;

namespace Assimp    {
    class BatchLoader;
    class Importer;
    class IOSystem;

    namespace LWS   {

// ---------------------------------------------------------------------------
/** Represents an element in a LWS file.
 *
 *  This can either be a single data line - <name> <value> or a data
 *  group - { name <data_line0> ... n }
 */
class Element
{
public:
    Element()
    {}

    // first: name, second: rest
    std::string tokens[2];
    std::list<Element> children;

    //! Recursive parsing function
    void Parse (const char*& buffer);
};

#define AI_LWS_MASK (0xffffffff >> 4u)

// ---------------------------------------------------------------------------
/** Represents a LWS scenegraph element
 */
struct NodeDesc
{
    NodeDesc()
        :   type()
        ,   id()
        ,   number  (0)
        ,   parent  (0)
        ,   name    ("")
        ,   isPivotSet (false)
        ,   lightColor (1.f,1.f,1.f)
        ,   lightIntensity (1.f)
        ,   lightType (0)
        ,   lightFalloffType (0)
        ,   lightConeAngle (45.f)
        ,   lightEdgeAngle()
        ,   parent_resolved (NULL)
    {}

    enum {

        OBJECT = 1,
        LIGHT  = 2,
        CAMERA = 3,
        BONE   = 4
    } type; // type of node

    // if object: path
    std::string path;
    unsigned int id;

    // number of object
    unsigned int number;

    // index of parent index
    unsigned int parent;

    // lights & cameras & dummies: name
    const char* name;

    // animation channels
    std::list< LWO::Envelope > channels;

    // position of pivot point
    aiVector3D pivotPos;
    bool isPivotSet;



    // color of light source
    aiColor3D lightColor;

    // intensity of light source
    float lightIntensity;

    // type of light source
    unsigned int lightType;

    // falloff type of light source
    unsigned int lightFalloffType;

    // cone angle of (spot) light source
    float lightConeAngle;

    // soft cone angle of (spot) light source
    float lightEdgeAngle;



    // list of resolved children
    std::list< NodeDesc* > children;

    // resolved parent node
    NodeDesc* parent_resolved;


    // for std::find()
    bool operator == (unsigned int num)  const {
        if (!num)
            return false;
        unsigned int _type = num >> 28u;

        return _type == static_cast<unsigned int>(type) && (num & AI_LWS_MASK) == number;
    }
};

} // end namespace LWS

// ---------------------------------------------------------------------------
/** LWS (LightWave Scene Format) importer class.
 *
 *  This class does heavily depend on the LWO importer class. LWS files
 *  contain mainly descriptions how LWO objects are composed together
 *  in a scene.
*/
class LWSImporter : public BaseImporter
{
public:
    LWSImporter();
    ~LWSImporter();


public:

    // -------------------------------------------------------------------
    // Check whether we can read a specific file
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool checkSig) const;

protected:

    // -------------------------------------------------------------------
    // Get list of supported extensions
    const aiImporterDesc* GetInfo () const;

    // -------------------------------------------------------------------
    // Import file into given scene data structure
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

    // -------------------------------------------------------------------
    // Setup import properties
    void SetupProperties(const Importer* pImp);

private:


    // -------------------------------------------------------------------
    // Read an envelope description
    void ReadEnvelope(const LWS::Element& dad, LWO::Envelope& out );

    // -------------------------------------------------------------------
    // Read an envelope description for the older LW file format
    void ReadEnvelope_Old(std::list< LWS::Element >::const_iterator& it,
        const std::list< LWS::Element >::const_iterator& end,
        LWS::NodeDesc& nodes,
        unsigned int version);

    // -------------------------------------------------------------------
    // Setup a nice name for a node
    void SetupNodeName(aiNode* nd, LWS::NodeDesc& src);

    // -------------------------------------------------------------------
    // Recursively build the scenegraph
    void BuildGraph(aiNode* nd,
        LWS::NodeDesc& src,
        std::vector<AttachmentInfo>& attach,
        BatchLoader& batch,
        aiCamera**& camOut,
        aiLight**& lightOut,
        std::vector<aiNodeAnim*>& animOut);

    // -------------------------------------------------------------------
    // Try several dirs until we find the right location of a LWS file.
    std::string FindLWOFile(const std::string& in);

private:

    bool configSpeedFlag;
    IOSystem* io;

    double first,last,fps;

    bool noSkeletonMesh;
};

} // end of namespace Assimp

#endif // AI_LWSIMPORTER_H_INC
