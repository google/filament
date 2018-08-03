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
/// \file   X3DImporter_Node.hpp
/// \brief  Elements of scene graph.
/// \date   2015-2016
/// \author smal.root@gmail.com

#ifndef INCLUDED_AI_X3D_IMPORTER_NODE_H
#define INCLUDED_AI_X3D_IMPORTER_NODE_H

// Header files, Assimp.
#include <assimp/scene.h>
#include <assimp/types.h>

// Header files, stdlib.
#include <list>
#include <vector>
#include <string>

/// \class CX3DImporter_NodeElement
/// Base class for elements of nodes.
class CX3DImporter_NodeElement
{
	/***********************************************/
	/******************** Types ********************/
	/***********************************************/

public:

	/// \enum EType
	/// Define what data type contain node element.
	enum EType
	{
		ENET_Group,                  ///< Element has type "Group".
		ENET_MetaBoolean,            ///< Element has type "Metadata boolean".
		ENET_MetaDouble,             ///< Element has type "Metadata double".
		ENET_MetaFloat,              ///< Element has type "Metadata float".
		ENET_MetaInteger,            ///< Element has type "Metadata integer".
		ENET_MetaSet,                ///< Element has type "Metadata set".
		ENET_MetaString,             ///< Element has type "Metadata string".
		ENET_Arc2D,                  ///< Element has type "Arc2D".
		ENET_ArcClose2D,             ///< Element has type "ArcClose2D".
		ENET_Circle2D,               ///< Element has type "Circle2D".
		ENET_Disk2D,                 ///< Element has type "Disk2D".
		ENET_Polyline2D,             ///< Element has type "Polyline2D".
		ENET_Polypoint2D,            ///< Element has type "Polypoint2D".
		ENET_Rectangle2D,            ///< Element has type "Rectangle2D".
		ENET_TriangleSet2D,          ///< Element has type "TriangleSet2D".
		ENET_Box,                    ///< Element has type "Box".
		ENET_Cone,                   ///< Element has type "Cone".
		ENET_Cylinder,               ///< Element has type "Cylinder".
		ENET_Sphere,                 ///< Element has type "Sphere".
		ENET_ElevationGrid,          ///< Element has type "ElevationGrid".
		ENET_Extrusion,              ///< Element has type "Extrusion".
		ENET_Coordinate,             ///< Element has type "Coordinate".
		ENET_Normal,                 ///< Element has type "Normal".
		ENET_TextureCoordinate,      ///< Element has type "TextureCoordinate".
		ENET_IndexedFaceSet,         ///< Element has type "IndexedFaceSet".
		ENET_IndexedLineSet,         ///< Element has type "IndexedLineSet".
		ENET_IndexedTriangleSet,     ///< Element has type "IndexedTriangleSet".
		ENET_IndexedTriangleFanSet,  ///< Element has type "IndexedTriangleFanSet".
		ENET_IndexedTriangleStripSet,///< Element has type "IndexedTriangleStripSet".
		ENET_LineSet,                ///< Element has type "LineSet".
		ENET_PointSet,               ///< Element has type "PointSet".
		ENET_TriangleSet,            ///< Element has type "TriangleSet".
		ENET_TriangleFanSet,         ///< Element has type "TriangleFanSet".
		ENET_TriangleStripSet,       ///< Element has type "TriangleStripSet".
		ENET_Color,                  ///< Element has type "Color".
		ENET_ColorRGBA,              ///< Element has type "ColorRGBA".
		ENET_Shape,                  ///< Element has type "Shape".
		ENET_Appearance,             ///< Element has type "Appearance".
		ENET_Material,               ///< Element has type "Material".
		ENET_ImageTexture,           ///< Element has type "ImageTexture".
		ENET_TextureTransform,       ///< Element has type "TextureTransform".
		ENET_DirectionalLight,       ///< Element has type "DirectionalLight".
		ENET_PointLight,             ///< Element has type "PointLight".
		ENET_SpotLight,              ///< Element has type "SpotLight".

		ENET_Invalid                 ///< Element has invalid type and possible contain invalid data.
	};

	/***********************************************/
	/****************** Constants ******************/
	/***********************************************/

public:

	const EType Type;

	/***********************************************/
	/****************** Variables ******************/
	/***********************************************/

public:

	std::string ID;///< ID of the element. Can be empty. In X3D synonym for "ID" attribute.
	CX3DImporter_NodeElement* Parent;///< Parent element. If nullptr then this node is root.
	std::list<CX3DImporter_NodeElement*> Child;///< Child elements.

	/***********************************************/
	/****************** Functions ******************/
	/***********************************************/

    /// @brief  The destructor, virtual.
    virtual ~CX3DImporter_NodeElement() {
        // empty
    }

private:
	/// Disabled copy constructor.
	CX3DImporter_NodeElement(const CX3DImporter_NodeElement& pNodeElement);

	/// Disabled assign operator.
	CX3DImporter_NodeElement& operator=(const CX3DImporter_NodeElement& pNodeElement);

	/// Disabled default constructor.
	CX3DImporter_NodeElement();

protected:
	/// In constructor inheritor must set element type.
	/// \param [in] pType - element type.
	/// \param [in] pParent - parent element.
	CX3DImporter_NodeElement(const EType pType, CX3DImporter_NodeElement* pParent)
		: Type(pType), Parent(pParent)
	{}
};// class IX3DImporter_NodeElement

/// \class CX3DImporter_NodeElement_Group
/// Class that define grouping node. Define transformation matrix for children.
/// Also can select which child will be kept and others are removed.
class CX3DImporter_NodeElement_Group : public CX3DImporter_NodeElement
{
	/***********************************************/
	/****************** Variables ******************/
	/***********************************************/

public:

	aiMatrix4x4 Transformation;///< Transformation matrix.

	/// \var bool Static
	/// As you know node elements can use already defined node elements when attribute "USE" is defined.
	/// Standard search when looking for an element in the whole scene graph, existing at this moment.
	/// If a node is marked as static, the children(or lower) can not search for elements in the nodes upper then static.
	bool Static;

	bool UseChoice;///< Flag: if true then use number from \ref Choice to choose what the child will be kept.
	int32_t Choice;///< Number of the child which will be kept.

	/***********************************************/
	/****************** Functions ******************/
	/***********************************************/

private:

	/// \fn CX3DImporter_NodeElement_Group(const CX3DImporter_NodeElement_Group& pNode)
	/// Disabled copy constructor.
	CX3DImporter_NodeElement_Group(const CX3DImporter_NodeElement_Group& pNode);

	/// \fn CX3DImporter_NodeElement_Group& operator=(const CX3DImporter_NodeElement_Group& pNode)
	/// Disabled assign operator.
	CX3DImporter_NodeElement_Group& operator=(const CX3DImporter_NodeElement_Group& pNode);

	/// \fn CX3DImporter_NodeElement_Group()
	/// Disabled default constructor.
	CX3DImporter_NodeElement_Group();

public:

	/// \fn CX3DImporter_NodeElement_Group(CX3DImporter_NodeElement_Group* pParent, const bool pStatic = false)
	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	/// \param [in] pStatic - static node flag.
	CX3DImporter_NodeElement_Group(CX3DImporter_NodeElement* pParent, const bool pStatic = false)
		: CX3DImporter_NodeElement(ENET_Group, pParent), Static(pStatic), UseChoice(false)
	{}

};// class CX3DImporter_NodeElement_Group

/// \class CX3DImporter_NodeElement_Meta
/// This struct describe metavalue.
class CX3DImporter_NodeElement_Meta : public CX3DImporter_NodeElement
{
	/***********************************************/
	/****************** Variables ******************/
	/***********************************************/

public:

	std::string Name;///< Name of metadata object.
	/// \var std::string Reference
	/// If provided, it identifies the metadata standard or other specification that defines the name field. If the reference field is not provided or is
	/// empty, the meaning of the name field is considered implicit to the characters in the string.
	std::string Reference;

	/***********************************************/
	/****************** Functions ******************/
	/***********************************************/

private:

	/// \fn CX3DImporter_NodeElement_Meta(const CX3DImporter_NodeElement_Meta& pNode)
	/// Disabled copy constructor.
	CX3DImporter_NodeElement_Meta(const CX3DImporter_NodeElement_Meta& pNode);

	/// \fn CX3DImporter_NodeElement_Meta& operator=(const CX3DImporter_NodeElement_Meta& pNode)
	/// Disabled assign operator.
	CX3DImporter_NodeElement_Meta& operator=(const CX3DImporter_NodeElement_Meta& pNode);

	/// \fn CX3DImporter_NodeElement_Meta()
	/// Disabled default constructor.
	CX3DImporter_NodeElement_Meta();

public:

	/// \fn CX3DImporter_NodeElement_Meta(const EType pType, CX3DImporter_NodeElement* pParent)
	/// In constructor inheritor must set element type.
	/// \param [in] pType - element type.
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_Meta(const EType pType, CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(pType, pParent)
	{}

};// class CX3DImporter_NodeElement_Meta

/// \struct CX3DImporter_NodeElement_MetaBoolean
/// This struct describe metavalue of type boolean.
struct CX3DImporter_NodeElement_MetaBoolean : public CX3DImporter_NodeElement_Meta
{
	std::vector<bool> Value;///< Stored value.

	/// \fn CX3DImporter_NodeElement_MetaBoolean(CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_MetaBoolean(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement_Meta(ENET_MetaBoolean, pParent)
	{}

};// struct CX3DImporter_NodeElement_MetaBoolean

/// \struct CX3DImporter_NodeElement_MetaDouble
/// This struct describe metavalue of type double.
struct CX3DImporter_NodeElement_MetaDouble : public CX3DImporter_NodeElement_Meta
{
	std::vector<double> Value;///< Stored value.

	/// \fn CX3DImporter_NodeElement_MetaDouble(CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_MetaDouble(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement_Meta(ENET_MetaDouble, pParent)
	{}

};// struct CX3DImporter_NodeElement_MetaDouble

/// \struct CX3DImporter_NodeElement_MetaFloat
/// This struct describe metavalue of type float.
struct CX3DImporter_NodeElement_MetaFloat : public CX3DImporter_NodeElement_Meta
{
	std::vector<float> Value;///< Stored value.

	/// \fn CX3DImporter_NodeElement_MetaFloat(CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_MetaFloat(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement_Meta(ENET_MetaFloat, pParent)
	{}

};// struct CX3DImporter_NodeElement_MetaFloat

/// \struct CX3DImporter_NodeElement_MetaInteger
/// This struct describe metavalue of type integer.
struct CX3DImporter_NodeElement_MetaInteger : public CX3DImporter_NodeElement_Meta
{
	std::vector<int32_t> Value;///< Stored value.

	/// \fn CX3DImporter_NodeElement_MetaInteger(CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_MetaInteger(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement_Meta(ENET_MetaInteger, pParent)
	{}

};// struct CX3DImporter_NodeElement_MetaInteger

/// \struct CX3DImporter_NodeElement_MetaSet
/// This struct describe container for metaobjects.
struct CX3DImporter_NodeElement_MetaSet : public CX3DImporter_NodeElement_Meta
{
	std::list<CX3DImporter_NodeElement_Meta> Value;///< Stored value.

	/// \fn CX3DImporter_NodeElement_MetaSet(CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_MetaSet(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement_Meta(ENET_MetaSet, pParent)
	{}

};// struct CX3DImporter_NodeElement_MetaSet

/// \struct CX3DImporter_NodeElement_MetaString
/// This struct describe metavalue of type string.
struct CX3DImporter_NodeElement_MetaString : public CX3DImporter_NodeElement_Meta
{
	std::list<std::string> Value;///< Stored value.

	/// \fn CX3DImporter_NodeElement_MetaString(CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_MetaString(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement_Meta(ENET_MetaString, pParent)
	{}

};// struct CX3DImporter_NodeElement_MetaString

/// \struct CX3DImporter_NodeElement_Color
/// This struct hold <Color> value.
struct CX3DImporter_NodeElement_Color : public CX3DImporter_NodeElement
{
	std::list<aiColor3D> Value;///< Stored value.

	/// \fn CX3DImporter_NodeElement_Color(CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_Color(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(ENET_Color, pParent)
	{}

};// struct CX3DImporter_NodeElement_Color

/// \struct CX3DImporter_NodeElement_ColorRGBA
/// This struct hold <ColorRGBA> value.
struct CX3DImporter_NodeElement_ColorRGBA : public CX3DImporter_NodeElement
{
	std::list<aiColor4D> Value;///< Stored value.

	/// \fn CX3DImporter_NodeElement_ColorRGBA(CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_ColorRGBA(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(ENET_ColorRGBA, pParent)
	{}

};// struct CX3DImporter_NodeElement_ColorRGBA

/// \struct CX3DImporter_NodeElement_Coordinate
/// This struct hold <Coordinate> value.
struct CX3DImporter_NodeElement_Coordinate : public CX3DImporter_NodeElement
{
	std::list<aiVector3D> Value;///< Stored value.

	/// \fn CX3DImporter_NodeElement_Coordinate(CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_Coordinate(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(ENET_Coordinate, pParent)
	{}

};// struct CX3DImporter_NodeElement_Coordinate

/// \struct CX3DImporter_NodeElement_Normal
/// This struct hold <Normal> value.
struct CX3DImporter_NodeElement_Normal : public CX3DImporter_NodeElement
{
	std::list<aiVector3D> Value;///< Stored value.

	/// \fn CX3DImporter_NodeElement_Normal(CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_Normal(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(ENET_Normal, pParent)
	{}

};// struct CX3DImporter_NodeElement_Normal

/// \struct CX3DImporter_NodeElement_TextureCoordinate
/// This struct hold <TextureCoordinate> value.
struct CX3DImporter_NodeElement_TextureCoordinate : public CX3DImporter_NodeElement
{
	std::list<aiVector2D> Value;///< Stored value.

	/// \fn CX3DImporter_NodeElement_TextureCoordinate(CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_TextureCoordinate(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(ENET_TextureCoordinate, pParent)
	{}

};// struct CX3DImporter_NodeElement_TextureCoordinate

/// \class CX3DImporter_NodeElement_Geometry2D
/// Two-dimensional figure.
class CX3DImporter_NodeElement_Geometry2D : public CX3DImporter_NodeElement
{
	/***********************************************/
	/****************** Variables ******************/
	/***********************************************/

public:

	std::list<aiVector3D> Vertices;///< Vertices list.
	size_t NumIndices;///< Number of indices in one face.
	bool Solid;///< Flag: if true then render must use back-face culling, else render must draw both sides of object.

	/***********************************************/
	/****************** Functions ******************/
	/***********************************************/

private:

	/// \fn CX3DImporter_NodeElement_Geometry2D(const CX3DImporter_NodeElement_Geometry2D& pNode)
	/// Disabled copy constructor.
	CX3DImporter_NodeElement_Geometry2D(const CX3DImporter_NodeElement_Geometry2D& pNode);

	/// \fn CX3DImporter_NodeElement_Geometry2D& operator=(const CX3DImporter_NodeElement_Geometry2D& pNode)
	/// Disabled assign operator.
	CX3DImporter_NodeElement_Geometry2D& operator=(const CX3DImporter_NodeElement_Geometry2D& pNode);

public:

	/// \fn CX3DImporter_NodeElement_Geometry2D(const EType pType, CX3DImporter_NodeElement* pParent)
	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	/// \param [in] pType - type of geometry object.
	CX3DImporter_NodeElement_Geometry2D(const EType pType, CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(pType, pParent), Solid(true)
	{}

};// class CX3DImporter_NodeElement_Geometry2D

/// \class CX3DImporter_NodeElement_Geometry3D
/// Three-dimensional body.
class CX3DImporter_NodeElement_Geometry3D : public CX3DImporter_NodeElement {
public:
	std::list<aiVector3D> Vertices;  ///< Vertices list.
	size_t                NumIndices;///< Number of indices in one face.
	bool                  Solid;     ///< Flag: if true then render must use back-face culling, else render must draw both sides of object.

	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	/// \param [in] pType - type of geometry object.
	CX3DImporter_NodeElement_Geometry3D(const EType pType, CX3DImporter_NodeElement* pParent)
	: CX3DImporter_NodeElement(pType, pParent)
	, Vertices()
	, NumIndices( 0 )
	, Solid(true) {
        // empty		
	}

private:
	/// Disabled copy constructor.
	CX3DImporter_NodeElement_Geometry3D(const CX3DImporter_NodeElement_Geometry3D& pNode);

	/// Disabled assign operator.
	CX3DImporter_NodeElement_Geometry3D& operator=(const CX3DImporter_NodeElement_Geometry3D& pNode);
};// class CX3DImporter_NodeElement_Geometry3D

/// \class CX3DImporter_NodeElement_ElevationGrid
/// Uniform rectangular grid of varying height.
class CX3DImporter_NodeElement_ElevationGrid : public CX3DImporter_NodeElement_Geometry3D
{
	/***********************************************/
	/****************** Variables ******************/
	/***********************************************/

public:

	bool NormalPerVertex;///< If true then normals are defined for every vertex, else for every face(line).
	bool ColorPerVertex;///< If true then colors are defined for every vertex, else for every face(line).
	/// \var CreaseAngle
	/// If the angle between the geometric normals of two adjacent faces is less than the crease angle, normals shall be calculated so that the faces are
	/// shaded smoothly across the edge; otherwise, normals shall be calculated so that a lighting discontinuity across the edge is produced.
	float CreaseAngle;
	std::vector<int32_t> CoordIdx;///< Coordinates list by faces. In X3D format: "-1" - delimiter for faces.

	/***********************************************/
	/****************** Functions ******************/
	/***********************************************/

private:

	/// \fn CX3DImporter_NodeElement_ElevationGrid(const CX3DImporter_NodeElement_ElevationGrid& pNode)
	/// Disabled copy constructor.
	CX3DImporter_NodeElement_ElevationGrid(const CX3DImporter_NodeElement_ElevationGrid& pNode);

	/// \fn CX3DImporter_NodeElement_ElevationGrid& operator=(const CX3DImporter_NodeElement_ElevationGrid& pNode)
	/// Disabled assign operator.
	CX3DImporter_NodeElement_ElevationGrid& operator=(const CX3DImporter_NodeElement_ElevationGrid& pNode);

public:

	/// \fn CX3DImporter_NodeElement_ElevationGrid(const EType pType, CX3DImporter_NodeElement* pParent)
	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	/// \param [in] pType - type of geometry object.
	CX3DImporter_NodeElement_ElevationGrid(const EType pType, CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement_Geometry3D(pType, pParent)
	{}

};// class CX3DImporter_NodeElement_IndexedSet

/// \class CX3DImporter_NodeElement_IndexedSet
/// Shape with indexed vertices.
class CX3DImporter_NodeElement_IndexedSet : public CX3DImporter_NodeElement_Geometry3D
{
	/***********************************************/
	/****************** Variables ******************/
	/***********************************************/

public:

	/// \var CCW
	/// The ccw field defines the ordering of the vertex coordinates of the geometry with respect to user-given or automatically generated normal vectors
	/// used in the lighting model equations. If ccw is TRUE, the normals shall follow the right hand rule; the orientation of each normal with respect to
	/// the vertices (taken in order) shall be such that the vertices appear to be oriented in a counterclockwise order when the vertices are viewed (in the
	/// local coordinate system of the Shape) from the opposite direction as the normal. If ccw is FALSE, the normals shall be oriented in the opposite
	/// direction. If normals are not generated but are supplied using a Normal node, and the orientation of the normals does not match the setting of the
	/// ccw field, results are undefined.
	bool CCW;
	std::vector<int32_t> ColorIndex;///< Field to specify the polygonal faces by indexing into the <Color> or <ColorRGBA>.
	bool ColorPerVertex;///< If true then colors are defined for every vertex, else for every face(line).
	/// \var Convex
	/// The convex field indicates whether all polygons in the shape are convex (TRUE). A polygon is convex if it is planar, does not intersect itself,
	/// and all of the interior angles at its vertices are less than 180 degrees. Non planar and self intersecting polygons may produce undefined results
	/// even if the convex field is FALSE.
	bool Convex;
	std::vector<int32_t> CoordIndex;///< Field to specify the polygonal faces by indexing into the <Coordinate>.
	/// \var CreaseAngle
	/// If the angle between the geometric normals of two adjacent faces is less than the crease angle, normals shall be calculated so that the faces are
	/// shaded smoothly across the edge; otherwise, normals shall be calculated so that a lighting discontinuity across the edge is produced.
	float CreaseAngle;
	std::vector<int32_t> NormalIndex;///< Field to specify the polygonal faces by indexing into the <Normal>.
	bool NormalPerVertex;///< If true then normals are defined for every vertex, else for every face(line).
	std::vector<int32_t> TexCoordIndex;///< Field to specify the polygonal faces by indexing into the <TextureCoordinate>.

	/***********************************************/
	/****************** Functions ******************/
	/***********************************************/

private:

	/// \fn CX3DImporter_NodeElement_IndexedSet(const CX3DImporter_NodeElement_IndexedSet& pNode)
	/// Disabled copy constructor.
	CX3DImporter_NodeElement_IndexedSet(const CX3DImporter_NodeElement_IndexedSet& pNode);

	/// \fn CX3DImporter_NodeElement_IndexedSet& operator=(const CX3DImporter_NodeElement_IndexedSet& pNode)
	/// Disabled assign operator.
	CX3DImporter_NodeElement_IndexedSet& operator=(const CX3DImporter_NodeElement_IndexedSet& pNode);

public:

	/// \fn CX3DImporter_NodeElement_IndexedSet(const EType pType, CX3DImporter_NodeElement* pParent)
	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	/// \param [in] pType - type of geometry object.
	CX3DImporter_NodeElement_IndexedSet(const EType pType, CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement_Geometry3D(pType, pParent)
	{}

};// class CX3DImporter_NodeElement_IndexedSet

/// \class CX3DImporter_NodeElement_Set
/// Shape with set of vertices.
class CX3DImporter_NodeElement_Set : public CX3DImporter_NodeElement_Geometry3D
{
	/***********************************************/
	/****************** Variables ******************/
	/***********************************************/

public:

	/// \var CCW
	/// The ccw field defines the ordering of the vertex coordinates of the geometry with respect to user-given or automatically generated normal vectors
	/// used in the lighting model equations. If ccw is TRUE, the normals shall follow the right hand rule; the orientation of each normal with respect to
	/// the vertices (taken in order) shall be such that the vertices appear to be oriented in a counterclockwise order when the vertices are viewed (in the
	/// local coordinate system of the Shape) from the opposite direction as the normal. If ccw is FALSE, the normals shall be oriented in the opposite
	/// direction. If normals are not generated but are supplied using a Normal node, and the orientation of the normals does not match the setting of the
	/// ccw field, results are undefined.
	bool CCW;
	bool ColorPerVertex;///< If true then colors are defined for every vertex, else for every face(line).
	bool NormalPerVertex;///< If true then normals are defined for every vertex, else for every face(line).
	std::vector<int32_t> CoordIndex;///< Field to specify the polygonal faces by indexing into the <Coordinate>.
	std::vector<int32_t> NormalIndex;///< Field to specify the polygonal faces by indexing into the <Normal>.
	std::vector<int32_t> TexCoordIndex;///< Field to specify the polygonal faces by indexing into the <TextureCoordinate>.
	std::vector<int32_t> VertexCount;///< Field describes how many vertices are to be used in each polyline(polygon) from the <Coordinate> field.

	/***********************************************/
	/****************** Functions ******************/
	/***********************************************/

private:

	/// \fn CX3DImporter_NodeElement_Set(const CX3DImporter_NodeElement_Set& pNode)
	/// Disabled copy constructor.
	CX3DImporter_NodeElement_Set(const CX3DImporter_NodeElement_Set& pNode);

	/// \fn CX3DImporter_NodeElement_Set& operator=(const CX3DImporter_NodeElement_Set& pNode)
	/// Disabled assign operator.
	CX3DImporter_NodeElement_Set& operator=(const CX3DImporter_NodeElement_Set& pNode);

public:

	/// \fn CX3DImporter_NodeElement_Set(const EType pType, CX3DImporter_NodeElement* pParent)
	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	/// \param [in] pType - type of geometry object.
	CX3DImporter_NodeElement_Set(const EType pType, CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement_Geometry3D(pType, pParent)
	{}

};// class CX3DImporter_NodeElement_Set

/// \struct CX3DImporter_NodeElement_Shape
/// This struct hold <Shape> value.
struct CX3DImporter_NodeElement_Shape : public CX3DImporter_NodeElement
{
	/// \fn CX3DImporter_NodeElement_Shape(CX3DImporter_NodeElement_Shape* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_Shape(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(ENET_Shape, pParent)
	{}

};// struct CX3DImporter_NodeElement_Shape

/// \struct CX3DImporter_NodeElement_Appearance
/// This struct hold <Appearance> value.
struct CX3DImporter_NodeElement_Appearance : public CX3DImporter_NodeElement
{
	/// \fn CX3DImporter_NodeElement_Appearance(CX3DImporter_NodeElement_Appearance* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_Appearance(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(ENET_Appearance, pParent)
	{}

};// struct CX3DImporter_NodeElement_Appearance

/// \class CX3DImporter_NodeElement_Material
/// Material.
class CX3DImporter_NodeElement_Material : public CX3DImporter_NodeElement {
public:
	float     AmbientIntensity;///< Specifies how much ambient light from light sources this surface shall reflect.
	aiColor3D DiffuseColor;    ///< Reflects all X3D light sources depending on the angle of the surface with respect to the light source.
	aiColor3D EmissiveColor;   ///< Models "glowing" objects. This can be useful for displaying pre-lit models.
	float     Shininess;       ///< Lower shininess values produce soft glows, while higher values result in sharper, smaller highlights.
	aiColor3D SpecularColor;   ///< The specularColor and shininess fields determine the specular highlights.
	float     Transparency;    ///< Specifies how "clear" an object is, with 1.0 being completely transparent, and 0.0 completely opaque.

	/// Constructor.
	/// \param [in] pParent - pointer to parent node.
	/// \param [in] pType - type of geometry object.
	CX3DImporter_NodeElement_Material(CX3DImporter_NodeElement* pParent)
	: CX3DImporter_NodeElement(ENET_Material, pParent)
	, AmbientIntensity( 0.0f )
	, DiffuseColor()
	, EmissiveColor()
	, Shininess( 0.0f )
	, SpecularColor()
	, Transparency( 1.0f ) {
		// empty
	}

private:
	/// Disabled copy constructor.
	CX3DImporter_NodeElement_Material(const CX3DImporter_NodeElement_Material& pNode);

	/// Disabled assign operator.
	CX3DImporter_NodeElement_Material& operator=(const CX3DImporter_NodeElement_Material& pNode);
};// class CX3DImporter_NodeElement_Material

/// \struct CX3DImporter_NodeElement_ImageTexture
/// This struct hold <ImageTexture> value.
struct CX3DImporter_NodeElement_ImageTexture : public CX3DImporter_NodeElement
{
	/// \var RepeatS
	/// RepeatS and RepeatT, that specify how the texture wraps in the S and T directions. If repeatS is TRUE (the default), the texture map is repeated
	/// outside the [0.0, 1.0] texture coordinate range in the S direction so that it fills the shape. If repeatS is FALSE, the texture coordinates are
	/// clamped in the S direction to lie within the [0.0, 1.0] range. The repeatT field is analogous to the repeatS field.
	bool RepeatS;
	bool RepeatT;///< See \ref RepeatS.
	std::string URL;///< URL of the texture.
	/// \fn CX3DImporter_NodeElement_ImageTexture(CX3DImporter_NodeElement_ImageTexture* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_ImageTexture(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(ENET_ImageTexture, pParent)
	{}

};// struct CX3DImporter_NodeElement_ImageTexture

/// \struct CX3DImporter_NodeElement_TextureTransform
/// This struct hold <TextureTransform> value.
struct CX3DImporter_NodeElement_TextureTransform : public CX3DImporter_NodeElement
{
	aiVector2D Center;///< Specifies a translation offset in texture coordinate space about which the rotation and scale fields are applied.
	float Rotation;///< Specifies a rotation in angle base units of the texture coordinates about the center point after the scale has been applied.
	aiVector2D Scale;///< Specifies a scaling factor in S and T of the texture coordinates about the center point.
	aiVector2D Translation;///<  Specifies a translation of the texture coordinates.

	/// \fn CX3DImporter_NodeElement_TextureTransform(CX3DImporter_NodeElement_TextureTransform* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	CX3DImporter_NodeElement_TextureTransform(CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(ENET_TextureTransform, pParent)
	{}

};// struct CX3DImporter_NodeElement_TextureTransform

/// \struct CX3DImporter_NodeElement_Light
/// This struct hold <TextureTransform> value.
struct CX3DImporter_NodeElement_Light : public CX3DImporter_NodeElement
{
	float AmbientIntensity;///< Specifies the intensity of the ambient emission from the light.
	aiColor3D Color;///< specifies the spectral colour properties of both the direct and ambient light emission as an RGB value.
	aiVector3D Direction;///< Specifies the direction vector of the illumination emanating from the light source in the local coordinate system.
	/// \var Global
	/// Field that determines whether the light is global or scoped. Global lights illuminate all objects that fall within their volume of lighting influence.
	/// Scoped lights only illuminate objects that are in the same transformation hierarchy as the light.
	bool Global;
	float Intensity;///< Specifies the brightness of the direct emission from the light.
	/// \var Attenuation
	/// PointLight node's illumination falls off with distance as specified by three attenuation coefficients. The attenuation factor
	/// is: "1 / max(attenuation[0] + attenuation[1] * r + attenuation[2] * r2, 1)", where r is the distance from the light to the surface being illuminated.
	aiVector3D Attenuation;
	aiVector3D Location;///< Specifies a translation offset of the centre point of the light source from the light's local coordinate system origin.
	float Radius;///< Specifies the radial extent of the solid angle and the maximum distance from location that may be illuminated by the light source.
	float BeamWidth;///< Specifies an inner solid angle in which the light source emits light at uniform full intensity.
	float CutOffAngle;///< The light source's emission intensity drops off from the inner solid angle (beamWidth) to the outer solid angle (cutOffAngle).

	/// \fn CX3DImporter_NodeElement_Light(EType pLightType, CX3DImporter_NodeElement* pParent)
	/// Constructor
	/// \param [in] pParent - pointer to parent node.
	/// \param [in] pLightType - type of the light source.
	CX3DImporter_NodeElement_Light(EType pLightType, CX3DImporter_NodeElement* pParent)
		: CX3DImporter_NodeElement(pLightType, pParent)
	{}

};// struct CX3DImporter_NodeElement_Light

#endif // INCLUDED_AI_X3D_IMPORTER_NODE_H
