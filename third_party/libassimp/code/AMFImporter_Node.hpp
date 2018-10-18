/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team



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

/// \file AMFImporter_Node.hpp
/// \brief Elements of scene graph.
/// \date 2016
/// \author smal.root@gmail.com

#pragma once
#ifndef INCLUDED_AI_AMF_IMPORTER_NODE_H
#define INCLUDED_AI_AMF_IMPORTER_NODE_H

// Header files, stdlib.
#include <list>
#include <string>
#include <vector>

// Header files, Assimp.
#include "assimp/types.h"
#include "assimp/scene.h"

/// \class CAMFImporter_NodeElement
/// Base class for elements of nodes.
class CAMFImporter_NodeElement {
public:
	/// Define what data type contain node element.
	enum EType {
		ENET_Color,        ///< Color element: <color>.
		ENET_Constellation,///< Grouping element: <constellation>.
		ENET_Coordinates,  ///< Coordinates element: <coordinates>.
		ENET_Edge,         ///< Edge element: <edge>.
		ENET_Instance,     ///< Grouping element: <constellation>.
		ENET_Material,     ///< Material element: <material>.
		ENET_Metadata,     ///< Metadata element: <metadata>.
		ENET_Mesh,         ///< Metadata element: <mesh>.
		ENET_Object,       ///< Element which hold object: <object>.
		ENET_Root,         ///< Root element: <amf>.
		ENET_Triangle,     ///< Triangle element: <triangle>.
		ENET_TexMap,       ///< Texture coordinates element: <texmap> or <map>.
		ENET_Texture,      ///< Texture element: <texture>.
		ENET_Vertex,       ///< Vertex element: <vertex>.
		ENET_Vertices,     ///< Vertex element: <vertices>.
		ENET_Volume,       ///< Volume element: <volume>.

		ENET_Invalid       ///< Element has invalid type and possible contain invalid data.
	};

	const EType Type;///< Type of element.
	std::string ID;///< ID of element.
	CAMFImporter_NodeElement* Parent;///< Parent element. If nullptr then this node is root.
	std::list<CAMFImporter_NodeElement*> Child;///< Child elements.

public:                                               /// Destructor, virtual..
    virtual ~CAMFImporter_NodeElement() {
        // empty
    }

	/// Disabled copy constructor and co.
	CAMFImporter_NodeElement(const CAMFImporter_NodeElement& pNodeElement) = delete;
    CAMFImporter_NodeElement(CAMFImporter_NodeElement&&) = delete;
    CAMFImporter_NodeElement& operator=(const CAMFImporter_NodeElement& pNodeElement) = delete;
	CAMFImporter_NodeElement() = delete;

protected:
	/// In constructor inheritor must set element type.
	/// \param [in] pType - element type.
	/// \param [in] pParent - parent element.
	CAMFImporter_NodeElement(const EType pType, CAMFImporter_NodeElement* pParent)
	: Type(pType)
    , ID()
    , Parent(pParent)
    , Child() {
        // empty
    }
};// class IAMFImporter_NodeElement

/// \struct CAMFImporter_NodeElement_Constellation
/// A collection of objects or constellations with specific relative locations.
struct CAMFImporter_NodeElement_Constellation : public CAMFImporter_NodeElement {
	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Constellation(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Constellation, pParent)
	{}

};// struct CAMFImporter_NodeElement_Constellation

/// \struct CAMFImporter_NodeElement_Instance
/// Part of constellation.
struct CAMFImporter_NodeElement_Instance : public CAMFImporter_NodeElement {

	std::string ObjectID;///< ID of object for instantiation.
	/// \var Delta - The distance of translation in the x, y, or z direction, respectively, in the referenced object's coordinate system, to
	/// create an instance of the object in the current constellation.
	aiVector3D Delta;

	/// \var Rotation - The rotation, in degrees, to rotate the referenced object about its x, y, and z axes, respectively, to create an
	/// instance of the object in the current constellation. Rotations shall be executed in order of x first, then y, then z.
	aiVector3D Rotation;

	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Instance(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Instance, pParent)
	{}
};

/// \struct CAMFImporter_NodeElement_Metadata
/// Structure that define metadata node.
struct CAMFImporter_NodeElement_Metadata : public CAMFImporter_NodeElement {

	std::string Type;///< Type of "Value". 
	std::string Value;///< Value.

	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Metadata(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Metadata, pParent)
	{}
};

/// \struct CAMFImporter_NodeElement_Root
/// Structure that define root node.
struct CAMFImporter_NodeElement_Root : public CAMFImporter_NodeElement {

	std::string Unit;///< The units to be used. May be "inch", "millimeter", "meter", "feet", or "micron".
	std::string Version;///< Version of format.

	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Root(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Root, pParent)
	{}
};

/// \struct CAMFImporter_NodeElement_Color
/// Structure that define object node.
struct CAMFImporter_NodeElement_Color : public CAMFImporter_NodeElement {
	bool Composed;                  ///< Type of color stored: if true then look for formula in \ref Color_Composed[4], else - in \ref Color.
	std::string Color_Composed[4];  ///< By components formulas of composed color. [0..3] - RGBA.
	aiColor4D Color;                ///< Constant color.
	std::string Profile;            ///< The ICC color space used to interpret the three color channels r, g and b..

	/// @brief  Constructor.
	/// @param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Color(CAMFImporter_NodeElement* pParent)
	: CAMFImporter_NodeElement(ENET_Color, pParent)
    , Composed( false )
    , Color()
    , Profile() {
        // empty
    }
};

/// \struct CAMFImporter_NodeElement_Material
/// Structure that define material node.
struct CAMFImporter_NodeElement_Material : public CAMFImporter_NodeElement {
	
	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Material(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Material, pParent)
	{}

};

/// \struct CAMFImporter_NodeElement_Object
/// Structure that define object node.
struct CAMFImporter_NodeElement_Object : public CAMFImporter_NodeElement {

    /// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Object(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Object, pParent)
	{}
};

/// \struct CAMFImporter_NodeElement_Mesh
/// Structure that define mesh node.
struct CAMFImporter_NodeElement_Mesh : public CAMFImporter_NodeElement {
	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Mesh(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Mesh, pParent)
	{}
};

/// \struct CAMFImporter_NodeElement_Vertex
/// Structure that define vertex node.
struct CAMFImporter_NodeElement_Vertex : public CAMFImporter_NodeElement {
	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Vertex(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Vertex, pParent)
	{}
};

/// \struct CAMFImporter_NodeElement_Edge
/// Structure that define edge node.
struct CAMFImporter_NodeElement_Edge : public CAMFImporter_NodeElement {
	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Edge(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Edge, pParent)
	{}

};

/// \struct CAMFImporter_NodeElement_Vertices
/// Structure that define vertices node.
struct CAMFImporter_NodeElement_Vertices : public CAMFImporter_NodeElement {
	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Vertices(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Vertices, pParent)
	{}
};

/// \struct CAMFImporter_NodeElement_Volume
/// Structure that define volume node.
struct CAMFImporter_NodeElement_Volume : public CAMFImporter_NodeElement {
	std::string MaterialID;///< Which material to use.
	std::string Type;///< What this volume describes can be “region” or “support”. If none specified, “object” is assumed.

	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Volume(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Volume, pParent)
	{}
};

/// \struct CAMFImporter_NodeElement_Coordinates
/// Structure that define coordinates node.
struct CAMFImporter_NodeElement_Coordinates : public CAMFImporter_NodeElement
{
	aiVector3D Coordinate;///< Coordinate.

	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Coordinates(CAMFImporter_NodeElement* pParent)
		: CAMFImporter_NodeElement(ENET_Coordinates, pParent)
	{}

};

/// \struct CAMFImporter_NodeElement_TexMap
/// Structure that define texture coordinates node.
struct CAMFImporter_NodeElement_TexMap : public CAMFImporter_NodeElement {
	aiVector3D TextureCoordinate[3];///< Texture coordinates.
	std::string TextureID_R;///< Texture ID for red color component.
	std::string TextureID_G;///< Texture ID for green color component.
	std::string TextureID_B;///< Texture ID for blue color component.
	std::string TextureID_A;///< Texture ID for alpha color component.

	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_TexMap(CAMFImporter_NodeElement* pParent)
	: CAMFImporter_NodeElement(ENET_TexMap, pParent)
    , TextureCoordinate{}
    , TextureID_R()
    , TextureID_G()
    , TextureID_B()
    , TextureID_A()	{
        // empty
    }
};

/// \struct CAMFImporter_NodeElement_Triangle
/// Structure that define triangle node.
struct CAMFImporter_NodeElement_Triangle : public CAMFImporter_NodeElement {
	size_t V[3];///< Triangle vertices.

	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Triangle(CAMFImporter_NodeElement* pParent)
	: CAMFImporter_NodeElement(ENET_Triangle, pParent) {
        // empty
    }
};

/// Structure that define texture node.
struct CAMFImporter_NodeElement_Texture : public CAMFImporter_NodeElement {
	size_t Width, Height, Depth;///< Size of the texture.
	std::vector<uint8_t> Data;///< Data of the texture.
	bool Tiled;

	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	CAMFImporter_NodeElement_Texture(CAMFImporter_NodeElement* pParent)
	: CAMFImporter_NodeElement(ENET_Texture, pParent)
    , Width( 0 )
    , Height( 0 )
    , Depth( 0 )
    , Data()
    , Tiled( false ){
        // empty
    }
};

#endif // INCLUDED_AI_AMF_IMPORTER_NODE_H
