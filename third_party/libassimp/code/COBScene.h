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

/** @file  COBScene.h
*  @brief Utilities for the COB importer.
*/
#ifndef INCLUDED_AI_COB_SCENE_H
#define INCLUDED_AI_COB_SCENE_H

#include <memory>
#include <deque>
#include <map>

#include <assimp/BaseImporter.h>
#include <assimp/material.h>

namespace Assimp {
namespace COB {

// ------------------
/** Represents a single vertex index in a face */
struct VertexIndex
{
    // intentionally uninitialized
    unsigned int pos_idx,uv_idx;
};

// ------------------
/** COB Face data structure */
struct Face
{
    // intentionally uninitialized
    unsigned int material, flags;
    std::vector<VertexIndex> indices;
};

// ------------------
/** COB chunk header information */
struct ChunkInfo
{
    enum {NO_SIZE=UINT_MAX};

    ChunkInfo ()
        :   id        (0)
        ,   parent_id (0)
        ,   version   (0)
        ,   size      (NO_SIZE)
    {}

    // Id of this chunk, unique within file
    unsigned int id;

    // and the corresponding parent
    unsigned int parent_id;

    // version. v1.23 becomes 123
    unsigned int version;

    // chunk size in bytes, only relevant for binary files
    // NO_SIZE is also valid.
    unsigned int size;
};

// ------------------
/** A node in the scenegraph */
struct Node : public ChunkInfo
{
    enum Type {
        TYPE_MESH,TYPE_GROUP,TYPE_LIGHT,TYPE_CAMERA,TYPE_BONE
    };

    virtual ~Node() {}
    Node(Type type) : type(type), unit_scale(1.f){}

    Type type;

    // used during resolving
    typedef std::deque<const Node*> ChildList;
    mutable ChildList temp_children;

    // unique name
    std::string name;

    // local mesh transformation
    aiMatrix4x4 transform;

    // scaling for this node to get to the metric system
    float unit_scale;
};

// ------------------
/** COB Mesh data structure */
struct Mesh : public Node
{
    using ChunkInfo::operator=;
    enum DrawFlags {
        SOLID = 0x1,
        TRANS = 0x2,
        WIRED = 0x4,
        BBOX  = 0x8,
        HIDE  = 0x10
    };

    Mesh()
        : Node(TYPE_MESH)
        , draw_flags(SOLID)
    {}

    // vertex elements
    std::vector<aiVector2D> texture_coords;
    std::vector<aiVector3D> vertex_positions;

    // face data
    std::vector<Face> faces;

    // misc. drawing flags
    unsigned int draw_flags;

    // used during resolving
    typedef std::deque<Face*> FaceRefList;
    typedef std::map< unsigned int,FaceRefList > TempMap;
    TempMap temp_map;
};

// ------------------
/** COB Group data structure */
struct Group : public Node
{
    using ChunkInfo::operator=;
    Group() : Node(TYPE_GROUP) {}
};

// ------------------
/** COB Bone data structure */
struct Bone : public Node
{
    using ChunkInfo::operator=;
    Bone() : Node(TYPE_BONE) {}
};

// ------------------
/** COB Light data structure */
struct Light : public Node
{
    enum LightType {
        SPOT,LOCAL,INFINITE
    };

    using ChunkInfo::operator=;
    Light() : Node(TYPE_LIGHT),angle(),inner_angle(),ltype(SPOT) {}

    aiColor3D color;
    float angle,inner_angle;

    LightType ltype;
};

// ------------------
/** COB Camera data structure */
struct Camera : public Node
{
    using ChunkInfo::operator=;
    Camera() : Node(TYPE_CAMERA) {}
};

// ------------------
/** COB Texture data structure */
struct Texture
{
    std::string path;
    aiUVTransform transform;
};

// ------------------
/** COB Material data structure */
struct Material : ChunkInfo
{
    using ChunkInfo::operator=;
    enum Shader {
        FLAT,PHONG,METAL
    };

    enum AutoFacet {
        FACETED,AUTOFACETED,SMOOTH
    };

    Material() : alpha(),exp(),ior(),ka(),ks(1.f),
        matnum(UINT_MAX),
        shader(FLAT),autofacet(FACETED),
        autofacet_angle()
    {}

    std::string type;

    aiColor3D rgb;
    float alpha, exp, ior,ka,ks;

    unsigned int matnum;
    Shader shader;

    AutoFacet autofacet;
    float autofacet_angle;

    std::shared_ptr<Texture> tex_env,tex_bump,tex_color;
};

// ------------------
/** Embedded bitmap, for instance for the thumbnail image */
struct Bitmap : ChunkInfo
{
    Bitmap() : orig_size() {}
    struct BitmapHeader
    {
    };

    BitmapHeader head;
    size_t orig_size;
    std::vector<char> buff_zipped;
};

typedef std::deque< std::shared_ptr<Node> > NodeList;
typedef std::vector< Material > MaterialList;

// ------------------
/** Represents a master COB scene, even if we loaded just a single COB file */
struct Scene
{
    NodeList nodes;
    MaterialList materials;

    // becomes *0 later
    Bitmap thumbnail;
};

    } // end COB
} // end Assimp

#endif
