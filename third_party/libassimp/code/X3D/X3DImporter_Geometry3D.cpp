/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team


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
/// \file   X3DImporter_Geometry3D.cpp
/// \brief  Parsing data from nodes of "Geometry3D" set of X3D.
/// \date   2015-2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_X3D_IMPORTER

#include "X3DImporter.hpp"
#include "X3DImporter_Macro.hpp"

// Header files, Assimp.
#include <assimp/StandardShapes.h>

namespace Assimp
{

// <Box
// DEF=""       ID
// USE=""       IDREF
// size="2 2 2" SFVec3f [initializeOnly]
// solid="true" SFBool  [initializeOnly]
// />
// The Box node specifies a rectangular parallelepiped box centred at (0, 0, 0) in the local coordinate system and aligned with the local coordinate axes.
// By default, the box measures 2 units in each dimension, from -1 to +1. The size field specifies the extents of the box along the X-, Y-, and Z-axes
// respectively and each component value shall be greater than zero.
void X3DImporter::ParseNode_Geometry3D_Box()
{
    std::string def, use;
    bool solid = true;
    aiVector3D size(2, 2, 2);
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_REF("size", size, XML_ReadNode_GetAttrVal_AsVec3f);
		MACRO_ATTRREAD_CHECK_RET("solid", solid, XML_ReadNode_GetAttrVal_AsBool);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Box, ne);
	}
	else
	{
		// create and if needed - define new geometry object.
		ne = new CX3DImporter_NodeElement_Geometry3D(CX3DImporter_NodeElement::ENET_Box, NodeElement_Cur);
		if(!def.empty()) ne->ID = def;

		GeometryHelper_MakeQL_RectParallelepiped(size, ((CX3DImporter_NodeElement_Geometry3D*)ne)->Vertices);// get quad list
		((CX3DImporter_NodeElement_Geometry3D*)ne)->Solid = solid;
		((CX3DImporter_NodeElement_Geometry3D*)ne)->NumIndices = 4;
		// check for X3DMetadataObject childs.
		if(!mReader->isEmptyElement())
			ParseNode_Metadata(ne, "Box");
		else
			NodeElement_Cur->Child.push_back(ne);// add made object as child to current element

		NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
	}// if(!use.empty()) else
}

// <Cone
// DEF=""           ID
// USE=""           IDREF
// bottom="true"    SFBool [initializeOnly]
// bottomRadius="1" SFloat [initializeOnly]
// height="2"       SFloat [initializeOnly]
// side="true"      SFBool [initializeOnly]
// solid="true"     SFBool [initializeOnly]
// />
void X3DImporter::ParseNode_Geometry3D_Cone()
{
    std::string use, def;
    bool bottom = true;
    float bottomRadius = 1;
    float height = 2;
    bool side = true;
    bool solid = true;
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("solid", solid, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("side", side, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("bottom", bottom, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("height", height, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_RET("bottomRadius", bottomRadius, XML_ReadNode_GetAttrVal_AsFloat);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Cone, ne);
	}
	else
	{
		const unsigned int tess = 30;///TODO: IME tessellation factor through ai_property

		std::vector<aiVector3D> tvec;// temp array for vertices.

		// create and if needed - define new geometry object.
		ne = new CX3DImporter_NodeElement_Geometry3D(CX3DImporter_NodeElement::ENET_Cone, NodeElement_Cur);
		if(!def.empty()) ne->ID = def;

		// make cone or parts according to flags.
		if(side)
		{
			StandardShapes::MakeCone(height, 0, bottomRadius, tess, tvec, !bottom);
		}
		else if(bottom)
		{
			StandardShapes::MakeCircle(bottomRadius, tess, tvec);
			height = -(height / 2);
			for(std::vector<aiVector3D>::iterator it = tvec.begin(); it != tvec.end(); ++it) it->y = height;// y - because circle made in oXZ.
		}

		// copy data from temp array
		for(std::vector<aiVector3D>::iterator it = tvec.begin(); it != tvec.end(); ++it) ((CX3DImporter_NodeElement_Geometry3D*)ne)->Vertices.push_back(*it);

		((CX3DImporter_NodeElement_Geometry3D*)ne)->Solid = solid;
		((CX3DImporter_NodeElement_Geometry3D*)ne)->NumIndices = 3;
		// check for X3DMetadataObject childs.
		if(!mReader->isEmptyElement())
			ParseNode_Metadata(ne, "Cone");
		else
			NodeElement_Cur->Child.push_back(ne);// add made object as child to current element

		NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
	}// if(!use.empty()) else
}

// <Cylinder
// DEF=""        ID
// USE=""        IDREF
// bottom="true" SFBool [initializeOnly]
// height="2"    SFloat [initializeOnly]
// radius="1"    SFloat [initializeOnly]
// side="true"   SFBool [initializeOnly]
// solid="true"  SFBool [initializeOnly]
// top="true"    SFBool [initializeOnly]
// />
void X3DImporter::ParseNode_Geometry3D_Cylinder()
{
    std::string use, def;
    bool bottom = true;
    float height = 2;
    float radius = 1;
    bool side = true;
    bool solid = true;
    bool top = true;
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("radius", radius, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_RET("solid", solid, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("bottom", bottom, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("top", top, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("side", side, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("height", height, XML_ReadNode_GetAttrVal_AsFloat);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Cylinder, ne);
	}
	else
	{
		const unsigned int tess = 30;///TODO: IME tessellation factor through ai_property

		std::vector<aiVector3D> tside;// temp array for vertices of side.
		std::vector<aiVector3D> tcir;// temp array for vertices of circle.

		// create and if needed - define new geometry object.
		ne = new CX3DImporter_NodeElement_Geometry3D(CX3DImporter_NodeElement::ENET_Cylinder, NodeElement_Cur);
		if(!def.empty()) ne->ID = def;

		// make cilynder or parts according to flags.
		if(side) StandardShapes::MakeCone(height, radius, radius, tess, tside, true);

		height /= 2;// height defined for whole cylinder, when creating top and bottom circle we are using just half of height.
		if(top || bottom) StandardShapes::MakeCircle(radius, tess, tcir);
		// copy data from temp arrays
		std::list<aiVector3D>& vlist = ((CX3DImporter_NodeElement_Geometry3D*)ne)->Vertices;// just short alias.

		for(std::vector<aiVector3D>::iterator it = tside.begin(); it != tside.end(); ++it) vlist.push_back(*it);

		if(top)
		{
			for(std::vector<aiVector3D>::iterator it = tcir.begin(); it != tcir.end(); ++it)
			{
				(*it).y = height;// y - because circle made in oXZ.
				vlist.push_back(*it);
			}
		}// if(top)

		if(bottom)
		{
			for(std::vector<aiVector3D>::iterator it = tcir.begin(); it != tcir.end(); ++it)
			{
				(*it).y = -height;// y - because circle made in oXZ.
				vlist.push_back(*it);
			}
		}// if(top)

		((CX3DImporter_NodeElement_Geometry3D*)ne)->Solid = solid;
		((CX3DImporter_NodeElement_Geometry3D*)ne)->NumIndices = 3;
		// check for X3DMetadataObject childs.
		if(!mReader->isEmptyElement())
			ParseNode_Metadata(ne, "Cylinder");
		else
			NodeElement_Cur->Child.push_back(ne);// add made object as child to current element

		NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
	}// if(!use.empty()) else
}

// <ElevationGrid
// DEF=""                 ID
// USE=""                 IDREF
// ccw="true"             SFBool  [initializeOnly]
// colorPerVertex="true"  SFBool  [initializeOnly]
// creaseAngle="0"        SFloat  [initializeOnly]
// height=""              MFloat  [initializeOnly]
// normalPerVertex="true" SFBool  [initializeOnly]
// solid="true"           SFBool  [initializeOnly]
// xDimension="0"         SFInt32 [initializeOnly]
// xSpacing="1.0"         SFloat  [initializeOnly]
// zDimension="0"         SFInt32 [initializeOnly]
// zSpacing="1.0"         SFloat  [initializeOnly]
// >
//   <!-- ColorNormalTexCoordContentModel -->
// ColorNormalTexCoordContentModel can contain Color (or ColorRGBA), Normal and TextureCoordinate, in any order. No more than one instance of any single
// node type is allowed. A ProtoInstance node (with the proper node type) can be substituted for any node in this content model.
// </ElevationGrid>
// The ElevationGrid node specifies a uniform rectangular grid of varying height in the Y=0 plane of the local coordinate system. The geometry is described
// by a scalar array of height values that specify the height of a surface above each point of the grid. The xDimension and zDimension fields indicate
// the number of elements of the grid height array in the X and Z directions. Both xDimension and zDimension shall be greater than or equal to zero.
// If either the xDimension or the zDimension is less than two, the ElevationGrid contains no quadrilaterals.
void X3DImporter::ParseNode_Geometry3D_ElevationGrid()
{
    std::string use, def;
    bool ccw = true;
    bool colorPerVertex = true;
    float creaseAngle = 0;
    std::vector<float> height;
    bool normalPerVertex = true;
    bool solid = true;
    int32_t xDimension = 0;
    float xSpacing = 1;
    int32_t zDimension = 0;
    float zSpacing = 1;
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("solid", solid, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("ccw", ccw, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("colorPerVertex", colorPerVertex, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("normalPerVertex", normalPerVertex, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("creaseAngle", creaseAngle, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_REF("height", height, XML_ReadNode_GetAttrVal_AsArrF);
		MACRO_ATTRREAD_CHECK_RET("xDimension", xDimension, XML_ReadNode_GetAttrVal_AsI32);
		MACRO_ATTRREAD_CHECK_RET("xSpacing", xSpacing, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_RET("zDimension", zDimension, XML_ReadNode_GetAttrVal_AsI32);
		MACRO_ATTRREAD_CHECK_RET("zSpacing", zSpacing, XML_ReadNode_GetAttrVal_AsFloat);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_ElevationGrid, ne);
	}
	else
	{
		if((xSpacing == 0.0f) || (zSpacing == 0.0f)) throw DeadlyImportError("Spacing in <ElevationGrid> must be grater than zero.");
		if((xDimension <= 0) || (zDimension <= 0)) throw DeadlyImportError("Dimension in <ElevationGrid> must be grater than zero.");
		if((size_t)(xDimension * zDimension) != height.size()) Throw_IncorrectAttrValue("Heights count must be equal to \"xDimension * zDimension\"");

		// create and if needed - define new geometry object.
		ne = new CX3DImporter_NodeElement_ElevationGrid(CX3DImporter_NodeElement::ENET_ElevationGrid, NodeElement_Cur);
		if(!def.empty()) ne->ID = def;

		CX3DImporter_NodeElement_ElevationGrid& grid_alias = *((CX3DImporter_NodeElement_ElevationGrid*)ne);// create alias for conveience

		{// create grid vertices list
			std::vector<float>::const_iterator he_it = height.begin();

			for(int32_t zi = 0; zi < zDimension; zi++)// rows
			{
				for(int32_t xi = 0; xi < xDimension; xi++)// columns
				{
					aiVector3D tvec(xSpacing * xi, *he_it, zSpacing * zi);

					grid_alias.Vertices.push_back(tvec);
					++he_it;
				}
			}
		}// END: create grid vertices list
		//
		// create faces list. In "coordIdx" format
		//
		// check if we have quads
		if((xDimension < 2) || (zDimension < 2))// only one element in dimension is set, create line set.
		{
			((CX3DImporter_NodeElement_ElevationGrid*)ne)->NumIndices = 2;// will be holded as line set.
			for(size_t i = 0, i_e = (grid_alias.Vertices.size() - 1); i < i_e; i++)
			{
				grid_alias.CoordIdx.push_back(static_cast<int32_t>(i));
				grid_alias.CoordIdx.push_back(static_cast<int32_t>(i + 1));
				grid_alias.CoordIdx.push_back(-1);
			}
		}
		else// two or more elements in every dimension is set. create quad set.
		{
			((CX3DImporter_NodeElement_ElevationGrid*)ne)->NumIndices = 4;
			for(int32_t fzi = 0, fzi_e = (zDimension - 1); fzi < fzi_e; fzi++)// rows
			{
				for(int32_t fxi = 0, fxi_e = (xDimension - 1); fxi < fxi_e; fxi++)// columns
				{
					// points direction in face.
					if(ccw)
					{
						// CCW:
						//	3 2
						//	0 1
						grid_alias.CoordIdx.push_back((fzi + 1) * xDimension + fxi);
						grid_alias.CoordIdx.push_back((fzi + 1) * xDimension + (fxi + 1));
						grid_alias.CoordIdx.push_back(fzi * xDimension + (fxi + 1));
						grid_alias.CoordIdx.push_back(fzi * xDimension + fxi);
					}
					else
					{
						// CW:
						//	0 1
						//	3 2
						grid_alias.CoordIdx.push_back(fzi * xDimension + fxi);
						grid_alias.CoordIdx.push_back(fzi * xDimension + (fxi + 1));
						grid_alias.CoordIdx.push_back((fzi + 1) * xDimension + (fxi + 1));
						grid_alias.CoordIdx.push_back((fzi + 1) * xDimension + fxi);
					}// if(ccw) else

					grid_alias.CoordIdx.push_back(-1);
				}// for(int32_t fxi = 0, fxi_e = (xDimension - 1); fxi < fxi_e; fxi++)
			}// for(int32_t fzi = 0, fzi_e = (zDimension - 1); fzi < fzi_e; fzi++)
		}// if((xDimension < 2) || (zDimension < 2)) else

		grid_alias.ColorPerVertex = colorPerVertex;
		grid_alias.NormalPerVertex = normalPerVertex;
		grid_alias.CreaseAngle = creaseAngle;
		grid_alias.Solid = solid;
        // check for child nodes
        if(!mReader->isEmptyElement())
        {
			ParseHelper_Node_Enter(ne);
			MACRO_NODECHECK_LOOPBEGIN("ElevationGrid");
				// check for X3DComposedGeometryNodes
				if(XML_CheckNode_NameEqual("Color")) { ParseNode_Rendering_Color(); continue; }
				if(XML_CheckNode_NameEqual("ColorRGBA")) { ParseNode_Rendering_ColorRGBA(); continue; }
				if(XML_CheckNode_NameEqual("Normal")) { ParseNode_Rendering_Normal(); continue; }
				if(XML_CheckNode_NameEqual("TextureCoordinate")) { ParseNode_Texturing_TextureCoordinate(); continue; }
				// check for X3DMetadataObject
				if(!ParseHelper_CheckRead_X3DMetadataObject()) XML_CheckNode_SkipUnsupported("ElevationGrid");

			MACRO_NODECHECK_LOOPEND("ElevationGrid");
			ParseHelper_Node_Exit();
		}// if(!mReader->isEmptyElement())
		else
		{
			NodeElement_Cur->Child.push_back(ne);// add made object as child to current element
		}// if(!mReader->isEmptyElement()) else

		NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
	}// if(!use.empty()) else
}

template<typename TVector>
static void GeometryHelper_Extrusion_CurveIsClosed(std::vector<TVector>& pCurve, const bool pDropTail, const bool pRemoveLastPoint, bool& pCurveIsClosed)
{
    size_t cur_sz = pCurve.size();

	pCurveIsClosed = false;
	// for curve with less than four points checking is have no sense,
	if(cur_sz < 4) return;

	for(size_t s = 3, s_e = cur_sz; s < s_e; s++)
	{
		// search for first point of duplicated part.
		if(pCurve[0] == pCurve[s])
		{
			bool found = true;

			// check if tail(indexed by b2) is duplicate of head(indexed by b1).
			for(size_t b1 = 1, b2 = (s + 1); b2 < cur_sz; b1++, b2++)
			{
				if(pCurve[b1] != pCurve[b2])
				{// points not match: clear flag and break loop.
					found = false;

					break;
				}
			}// for(size_t b1 = 1, b2 = (s + 1); b2 < cur_sz; b1++, b2++)

			// if duplicate tail is found then drop or not it depending on flags.
			if(found)
			{
				pCurveIsClosed = true;
				if(pDropTail)
				{
					if(!pRemoveLastPoint) s++;// prepare value for iterator's arithmetics.

					pCurve.erase(pCurve.begin() + s, pCurve.end());// remove tail
				}

				break;
			}// if(found)
		}// if(pCurve[0] == pCurve[s])
	}// for(size_t s = 3, s_e = (cur_sz - 1); s < s_e; s++)
}

static aiVector3D GeometryHelper_Extrusion_GetNextY(const size_t pSpine_PointIdx, const std::vector<aiVector3D>& pSpine, const bool pSpine_Closed)
{
    const size_t spine_idx_last = pSpine.size() - 1;
    aiVector3D tvec;

	if((pSpine_PointIdx == 0) || (pSpine_PointIdx == spine_idx_last))// at first special cases
	{
		if(pSpine_Closed)
		{// If the spine curve is closed: The SCP for the first and last points is the same and is found using (spine[1] - spine[n - 2]) to compute the Y-axis.
			// As we even for closed spine curve last and first point in pSpine are not the same: duplicates(spine[n - 1] which are equivalent to spine[0])
			// in tail are removed.
			// So, last point in pSpine is a spine[n - 2]
			tvec = pSpine[1] - pSpine[spine_idx_last];
		}
		else if(pSpine_PointIdx == 0)
		{// The Y-axis used for the first point is the vector from spine[0] to spine[1]
			tvec = pSpine[1] - pSpine[0];
		}
		else
		{// The Y-axis used for the last point it is the vector from spine[n-2] to spine[n-1]. In our case(see above about dropping tail) spine[n - 1] is
			// the spine[0].
			tvec = pSpine[spine_idx_last] - pSpine[spine_idx_last - 1];
		}
	}// if((pSpine_PointIdx == 0) || (pSpine_PointIdx == spine_idx_last))
	else
	{// For all points other than the first or last: The Y-axis for spine[i] is found by normalizing the vector defined by (spine[i+1] - spine[i-1]).
		tvec = pSpine[pSpine_PointIdx + 1] - pSpine[pSpine_PointIdx - 1];
	}// if((pSpine_PointIdx == 0) || (pSpine_PointIdx == spine_idx_last)) else

	return tvec.Normalize();
}

static aiVector3D GeometryHelper_Extrusion_GetNextZ(const size_t pSpine_PointIdx, const std::vector<aiVector3D>& pSpine, const bool pSpine_Closed,
													const aiVector3D pVecZ_Prev)
{
    const aiVector3D zero_vec(0);
    const size_t spine_idx_last = pSpine.size() - 1;

    aiVector3D tvec;

	// at first special cases
	if(pSpine.size() < 3)// spine have not enough points for vector calculations.
	{
		tvec.Set(0, 0, 1);
	}
	else if(pSpine_PointIdx == 0)// special case: first point
	{
		if(pSpine_Closed)// for calculating use previous point in curve s[n - 2]. In list it's a last point, because point s[n - 1] was removed as duplicate.
		{
			tvec = (pSpine[1] - pSpine[0]) ^ (pSpine[spine_idx_last] - pSpine[0]);
		}
		else // for not closed curve first and next point(s[0] and s[1]) has the same vector Z.
		{
			bool found = false;

			// As said: "If the Z-axis of the first point is undefined (because the spine is not closed and the first two spine segments are collinear)
			// then the Z-axis for the first spine point with a defined Z-axis is used."
			// Walk through spine and find Z.
			for(size_t next_point = 2; (next_point <= spine_idx_last) && !found; next_point++)
			{
				// (pSpine[2] - pSpine[1]) ^ (pSpine[0] - pSpine[1])
				tvec = (pSpine[next_point] - pSpine[next_point - 1]) ^ (pSpine[next_point - 2] - pSpine[next_point - 1]);
				found = !tvec.Equal(zero_vec);
			}

			// if entire spine are collinear then use OZ axis.
			if(!found) tvec.Set(0, 0, 1);
		}// if(pSpine_Closed) else
	}// else if(pSpine_PointIdx == 0)
	else if(pSpine_PointIdx == spine_idx_last)// special case: last point
	{
		if(pSpine_Closed)
		{// do not forget that real last point s[n - 1] is removed as duplicated. And in this case we are calculating vector Z for point s[n - 2].
			tvec = (pSpine[0] - pSpine[pSpine_PointIdx]) ^ (pSpine[pSpine_PointIdx - 1] - pSpine[pSpine_PointIdx]);
			// if taken spine vectors are collinear then use previous vector Z.
			if(tvec.Equal(zero_vec)) tvec = pVecZ_Prev;
		}
		else
		{// vector Z for last point of not closed curve is previous vector Z.
			tvec = pVecZ_Prev;
		}
	}
	else// regular point
	{
		tvec = (pSpine[pSpine_PointIdx + 1] - pSpine[pSpine_PointIdx]) ^ (pSpine[pSpine_PointIdx - 1] - pSpine[pSpine_PointIdx]);
		// if taken spine vectors are collinear then use previous vector Z.
		if(tvec.Equal(zero_vec)) tvec = pVecZ_Prev;
	}

	// After determining the Z-axis, its dot product with the Z-axis of the previous spine point is computed. If this value is negative, the Z-axis
	// is flipped (multiplied by -1).
	if((tvec * pVecZ_Prev) < 0) tvec = -tvec;

	return tvec.Normalize();
}

// <Extrusion
// DEF=""                                 ID
// USE=""                                 IDREF
// beginCap="true"                        SFBool     [initializeOnly]
// ccw="true"                             SFBool     [initializeOnly]
// convex="true"                          SFBool     [initializeOnly]
// creaseAngle="0.0"                      SFloat     [initializeOnly]
// crossSection="1 1 1 -1 -1 -1 -1 1 1 1" MFVec2f    [initializeOnly]
// endCap="true"                          SFBool     [initializeOnly]
// orientation="0 0 1 0"                  MFRotation [initializeOnly]
// scale="1 1"                            MFVec2f    [initializeOnly]
// solid="true"                           SFBool     [initializeOnly]
// spine="0 0 0 0 1 0"                    MFVec3f    [initializeOnly]
// />
void X3DImporter::ParseNode_Geometry3D_Extrusion()
{
    std::string use, def;
    bool beginCap = true;
    bool ccw = true;
    bool convex = true;
    float creaseAngle = 0;
    std::vector<aiVector2D> crossSection;
    bool endCap = true;
    std::vector<float> orientation;
    std::vector<aiVector2D> scale;
    bool solid = true;
    std::vector<aiVector3D> spine;
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("beginCap", beginCap, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("ccw", ccw, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("convex", convex, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("creaseAngle", creaseAngle, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_REF("crossSection", crossSection, XML_ReadNode_GetAttrVal_AsArrVec2f);
		MACRO_ATTRREAD_CHECK_RET("endCap", endCap, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_REF("orientation", orientation, XML_ReadNode_GetAttrVal_AsArrF);
		MACRO_ATTRREAD_CHECK_REF("scale", scale, XML_ReadNode_GetAttrVal_AsArrVec2f);
		MACRO_ATTRREAD_CHECK_RET("solid", solid, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_REF("spine", spine, XML_ReadNode_GetAttrVal_AsArrVec3f);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Extrusion, ne);
	}
	else
	{
		//
		// check if default values must be assigned
		//
		if(spine.size() == 0)
		{
			spine.resize(2);
			spine[0].Set(0, 0, 0), spine[1].Set(0, 1, 0);
		}
		else if(spine.size() == 1)
		{
			throw DeadlyImportError("ParseNode_Geometry3D_Extrusion. Spine must have at least two points.");
		}

		if(crossSection.size() == 0)
		{
			crossSection.resize(5);
			crossSection[0].Set(1, 1), crossSection[1].Set(1, -1), crossSection[2].Set(-1, -1), crossSection[3].Set(-1, 1), crossSection[4].Set(1, 1);
		}

		{// orientation
			size_t ori_size = orientation.size() / 4;

			if(ori_size < spine.size())
			{
				float add_ori[4];// values that will be added

				if(ori_size == 1)// if "orientation" has one element(means one MFRotation with four components) then use it value for all spine points.
				{
					add_ori[0] = orientation[0], add_ori[1] = orientation[1], add_ori[2] = orientation[2], add_ori[3] = orientation[3];
				}
				else// else - use default values
				{
					add_ori[0] = 0, add_ori[1] = 0, add_ori[2] = 1, add_ori[3] = 0;
				}

				orientation.reserve(spine.size() * 4);
				for(size_t i = 0, i_e = (spine.size() - ori_size); i < i_e; i++)
					orientation.push_back(add_ori[0]), orientation.push_back(add_ori[1]), orientation.push_back(add_ori[2]), orientation.push_back(add_ori[3]);
			}

			if(orientation.size() % 4) throw DeadlyImportError("Attribute \"orientation\" in <Extrusion> must has multiple four quantity of numbers.");
		}// END: orientation

		{// scale
			if(scale.size() < spine.size())
			{
				aiVector2D add_sc;

				if(scale.size() == 1)// if "scale" has one element then use it value for all spine points.
					add_sc = scale[0];
				else// else - use default values
					add_sc.Set(1, 1);

				scale.reserve(spine.size());
				for(size_t i = 0, i_e = (spine.size() - scale.size()); i < i_e; i++) scale.push_back(add_sc);
			}
		}// END: scale
		//
		// create and if needed - define new geometry object.
		//
		ne = new CX3DImporter_NodeElement_IndexedSet(CX3DImporter_NodeElement::ENET_Extrusion, NodeElement_Cur);
		if(!def.empty()) ne->ID = def;

		CX3DImporter_NodeElement_IndexedSet& ext_alias = *((CX3DImporter_NodeElement_IndexedSet*)ne);// create alias for conveience
		// assign part of input data
		ext_alias.CCW = ccw;
		ext_alias.Convex = convex;
		ext_alias.CreaseAngle = creaseAngle;
		ext_alias.Solid = solid;

		//
		// How we done it at all?
		// 1. At first we will calculate array of basises for every point in spine(look SCP in ISO-dic). Also "orientation" vector
		// are applied vor every basis.
		// 2. After that we can create array of point sets: which are scaled, transferred to basis of relative basis and at final translated to real position
		// using relative spine point.
		// 3. Next step is creating CoordIdx array(do not forget "-1" delimiter). While creating CoordIdx also created faces for begin and end caps, if
		// needed. While createing CootdIdx is taking in account CCW flag.
		// 4. The last step: create Vertices list.
		//
        bool spine_closed;// flag: true if spine curve is closed.
        bool cross_closed;// flag: true if cross curve is closed.
        std::vector<aiMatrix3x3> basis_arr;// array of basises. ROW_a - X, ROW_b - Y, ROW_c - Z.
        std::vector<std::vector<aiVector3D> > pointset_arr;// array of point sets: cross curves.

        // detect closed curves
        GeometryHelper_Extrusion_CurveIsClosed(crossSection, true, true, cross_closed);// true - drop tail, true - remove duplicate end.
        GeometryHelper_Extrusion_CurveIsClosed(spine, true, true, spine_closed);// true - drop tail, true - remove duplicate end.
        // If both cap are requested and spine curve is closed then we can make only one cap. Because second cap will be the same surface.
        if(spine_closed)
        {
			beginCap |= endCap;
			endCap = false;
		}

        {// 1. Calculate array of basises.
			aiMatrix4x4 rotmat;
			aiVector3D vecX(0), vecY(0), vecZ(0);

			basis_arr.resize(spine.size());
			for(size_t i = 0, i_e = spine.size(); i < i_e; i++)
			{
				aiVector3D tvec;

				// get axises of basis.
				vecY = GeometryHelper_Extrusion_GetNextY(i, spine, spine_closed);
				vecZ = GeometryHelper_Extrusion_GetNextZ(i, spine, spine_closed, vecZ);
				vecX = (vecY ^ vecZ).Normalize();
				// get rotation matrix and apply "orientation" to basis
				aiMatrix4x4::Rotation(orientation[i * 4 + 3], aiVector3D(orientation[i * 4], orientation[i * 4 + 1], orientation[i * 4 + 2]), rotmat);
				tvec = vecX, tvec *= rotmat, basis_arr[i].a1 = tvec.x, basis_arr[i].a2 = tvec.y, basis_arr[i].a3 = tvec.z;
				tvec = vecY, tvec *= rotmat, basis_arr[i].b1 = tvec.x, basis_arr[i].b2 = tvec.y, basis_arr[i].b3 = tvec.z;
				tvec = vecZ, tvec *= rotmat, basis_arr[i].c1 = tvec.x, basis_arr[i].c2 = tvec.y, basis_arr[i].c3 = tvec.z;
			}// for(size_t i = 0, i_e = spine.size(); i < i_e; i++)
		}// END: 1. Calculate array of basises

		{// 2. Create array of point sets.
			aiMatrix4x4 scmat;
			std::vector<aiVector3D> tcross(crossSection.size());

			pointset_arr.resize(spine.size());
			for(size_t spi = 0, spi_e = spine.size(); spi < spi_e; spi++)
			{
				aiVector3D tc23vec;

				tc23vec.Set(scale[spi].x, 0, scale[spi].y);
				aiMatrix4x4::Scaling(tc23vec, scmat);
				for(size_t cri = 0, cri_e = crossSection.size(); cri < cri_e; cri++)
				{
					aiVector3D tvecX, tvecY, tvecZ;

					tc23vec.Set(crossSection[cri].x, 0, crossSection[cri].y);
					// apply scaling to point
					tcross[cri] = scmat * tc23vec;
					//
					// transfer point to new basis
					// calculate coordinate in new basis
					tvecX.Set(basis_arr[spi].a1, basis_arr[spi].a2, basis_arr[spi].a3), tvecX *= tcross[cri].x;
					tvecY.Set(basis_arr[spi].b1, basis_arr[spi].b2, basis_arr[spi].b3), tvecY *= tcross[cri].y;
					tvecZ.Set(basis_arr[spi].c1, basis_arr[spi].c2, basis_arr[spi].c3), tvecZ *= tcross[cri].z;
					// apply new coordinates and translate it to spine point.
					tcross[cri] = tvecX + tvecY + tvecZ + spine[spi];
				}// for(size_t cri = 0, cri_e = crossSection.size(); cri < cri_e; i++)

				pointset_arr[spi] = tcross;// store transferred point set
			}// for(size_t spi = 0, spi_e = spine.size(); spi < spi_e; i++)
		}// END: 2. Create array of point sets.

		{// 3. Create CoordIdx.
			// add caps if needed
			if(beginCap)
			{
				// add cap as polygon. vertices of cap are places at begin, so just add numbers from zero.
				for(size_t i = 0, i_e = crossSection.size(); i < i_e; i++) ext_alias.CoordIndex.push_back(static_cast<int32_t>(i));

				// add delimiter
				ext_alias.CoordIndex.push_back(-1);
			}// if(beginCap)

			if(endCap)
			{
				// add cap as polygon. vertices of cap are places at end, as for beginCap use just sequence of numbers but with offset.
				size_t beg = (pointset_arr.size() - 1) * crossSection.size();

				for(size_t i = beg, i_e = (beg + crossSection.size()); i < i_e; i++) ext_alias.CoordIndex.push_back(static_cast<int32_t>(i));

				// add delimiter
				ext_alias.CoordIndex.push_back(-1);
			}// if(beginCap)

			// add quads
			for(size_t spi = 0, spi_e = (spine.size() - 1); spi <= spi_e; spi++)
			{
				const size_t cr_sz = crossSection.size();
				const size_t cr_last = crossSection.size() - 1;

				size_t right_col;// hold index basis for points of quad placed in right column;

				if(spi != spi_e)
					right_col = spi + 1;
				else if(spine_closed)// if spine curve is closed then one more quad is needed: between first and last points of curve.
					right_col = 0;
				else
					break;// if spine curve is not closed then break the loop, because spi is out of range for that type of spine.

				for(size_t cri = 0; cri < cr_sz; cri++)
				{
					if(cri != cr_last)
					{
						MACRO_FACE_ADD_QUAD(ccw, ext_alias.CoordIndex,
											static_cast<int32_t>(spi * cr_sz + cri), 
                                            static_cast<int32_t>(right_col * cr_sz + cri), 
                                            static_cast<int32_t>(right_col * cr_sz + cri + 1), 
                                            static_cast<int32_t>(spi * cr_sz + cri + 1));
						// add delimiter
						ext_alias.CoordIndex.push_back(-1);
					}
					else if(cross_closed)// if cross curve is closed then one more quad is needed: between first and last points of curve.
					{
						MACRO_FACE_ADD_QUAD(ccw, ext_alias.CoordIndex,
                                            static_cast<int32_t>(spi * cr_sz + cri), 
                                            static_cast<int32_t>(right_col * cr_sz + cri), 
                                            static_cast<int32_t>(right_col * cr_sz + 0), 
                                            static_cast<int32_t>(spi * cr_sz + 0));
						// add delimiter
						ext_alias.CoordIndex.push_back(-1);
					}
				}// for(size_t cri = 0; cri < cr_sz; cri++)
			}// for(size_t spi = 0, spi_e = (spine.size() - 2); spi < spi_e; spi++)
		}// END: 3. Create CoordIdx.

		{// 4. Create vertices list.
			// just copy all vertices
			for(size_t spi = 0, spi_e = spine.size(); spi < spi_e; spi++)
			{
				for(size_t cri = 0, cri_e = crossSection.size(); cri < cri_e; cri++)
				{
					ext_alias.Vertices.push_back(pointset_arr[spi][cri]);
				}
			}
		}// END: 4. Create vertices list.
//PrintVectorSet("Ext. CoordIdx", ext_alias.CoordIndex);
//PrintVectorSet("Ext. Vertices", ext_alias.Vertices);
		// check for child nodes
		if(!mReader->isEmptyElement())
			ParseNode_Metadata(ne, "Extrusion");
		else
			NodeElement_Cur->Child.push_back(ne);// add made object as child to current element

		NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
	}// if(!use.empty()) else
}

// <IndexedFaceSet
// DEF=""                         ID
// USE=""                         IDREF
// ccw="true"             SFBool  [initializeOnly]
// colorIndex=""          MFInt32 [initializeOnly]
// colorPerVertex="true"  SFBool  [initializeOnly]
// convex="true"          SFBool  [initializeOnly]
// coordIndex=""          MFInt32 [initializeOnly]
// creaseAngle="0"        SFFloat [initializeOnly]
// normalIndex=""         MFInt32 [initializeOnly]
// normalPerVertex="true" SFBool  [initializeOnly]
// solid="true"           SFBool  [initializeOnly]
// texCoordIndex=""       MFInt32 [initializeOnly]
// >
//    <!-- ComposedGeometryContentModel -->
// ComposedGeometryContentModel is the child-node content model corresponding to X3DComposedGeometryNodes. It can contain Color (or ColorRGBA), Coordinate,
// Normal and TextureCoordinate, in any order. No more than one instance of these nodes is allowed. Multiple VertexAttribute (FloatVertexAttribute,
// Matrix3VertexAttribute, Matrix4VertexAttribute) nodes can also be contained.
// A ProtoInstance node (with the proper node type) can be substituted for any node in this content model.
// </IndexedFaceSet>
void X3DImporter::ParseNode_Geometry3D_IndexedFaceSet()
{
    std::string use, def;
    bool ccw = true;
    std::vector<int32_t> colorIndex;
    bool colorPerVertex = true;
    bool convex = true;
    std::vector<int32_t> coordIndex;
    float creaseAngle = 0;
    std::vector<int32_t> normalIndex;
    bool normalPerVertex = true;
    bool solid = true;
    std::vector<int32_t> texCoordIndex;
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("ccw", ccw, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_REF("colorIndex", colorIndex, XML_ReadNode_GetAttrVal_AsArrI32);
		MACRO_ATTRREAD_CHECK_RET("colorPerVertex", colorPerVertex, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("convex", convex, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_REF("coordIndex", coordIndex, XML_ReadNode_GetAttrVal_AsArrI32);
		MACRO_ATTRREAD_CHECK_RET("creaseAngle", creaseAngle, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_REF("normalIndex", normalIndex, XML_ReadNode_GetAttrVal_AsArrI32);
		MACRO_ATTRREAD_CHECK_RET("normalPerVertex", normalPerVertex, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("solid", solid, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_REF("texCoordIndex", texCoordIndex, XML_ReadNode_GetAttrVal_AsArrI32);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_IndexedFaceSet, ne);
	}
	else
	{
		// check data
		if(coordIndex.size() == 0) throw DeadlyImportError("IndexedFaceSet must contain not empty \"coordIndex\" attribute.");

		// create and if needed - define new geometry object.
		ne = new CX3DImporter_NodeElement_IndexedSet(CX3DImporter_NodeElement::ENET_IndexedFaceSet, NodeElement_Cur);
		if(!def.empty()) ne->ID = def;

		CX3DImporter_NodeElement_IndexedSet& ne_alias = *((CX3DImporter_NodeElement_IndexedSet*)ne);

		ne_alias.CCW = ccw;
		ne_alias.ColorIndex = colorIndex;
		ne_alias.ColorPerVertex = colorPerVertex;
		ne_alias.Convex = convex;
		ne_alias.CoordIndex = coordIndex;
		ne_alias.CreaseAngle = creaseAngle;
		ne_alias.NormalIndex = normalIndex;
		ne_alias.NormalPerVertex = normalPerVertex;
		ne_alias.Solid = solid;
		ne_alias.TexCoordIndex = texCoordIndex;
        // check for child nodes
        if(!mReader->isEmptyElement())
        {
			ParseHelper_Node_Enter(ne);
			MACRO_NODECHECK_LOOPBEGIN("IndexedFaceSet");
				// check for X3DComposedGeometryNodes
				if(XML_CheckNode_NameEqual("Color")) { ParseNode_Rendering_Color(); continue; }
				if(XML_CheckNode_NameEqual("ColorRGBA")) { ParseNode_Rendering_ColorRGBA(); continue; }
				if(XML_CheckNode_NameEqual("Coordinate")) { ParseNode_Rendering_Coordinate(); continue; }
				if(XML_CheckNode_NameEqual("Normal")) { ParseNode_Rendering_Normal(); continue; }
				if(XML_CheckNode_NameEqual("TextureCoordinate")) { ParseNode_Texturing_TextureCoordinate(); continue; }
				// check for X3DMetadataObject
				if(!ParseHelper_CheckRead_X3DMetadataObject()) XML_CheckNode_SkipUnsupported("IndexedFaceSet");

			MACRO_NODECHECK_LOOPEND("IndexedFaceSet");
			ParseHelper_Node_Exit();
		}// if(!mReader->isEmptyElement())
		else
		{
			NodeElement_Cur->Child.push_back(ne);// add made object as child to current element
		}

		NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
	}// if(!use.empty()) else
}

// <Sphere
// DEF=""       ID
// USE=""       IDREF
// radius="1"   SFloat [initializeOnly]
// solid="true" SFBool [initializeOnly]
// />
void X3DImporter::ParseNode_Geometry3D_Sphere()
{
    std::string use, def;
    ai_real radius = 1;
    bool solid = true;
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("radius", radius, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_RET("solid", solid, XML_ReadNode_GetAttrVal_AsBool);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Sphere, ne);
	}
	else
	{
		const unsigned int tess = 3;///TODO: IME tessellation factor through ai_property

		std::vector<aiVector3D> tlist;

		// create and if needed - define new geometry object.
		ne = new CX3DImporter_NodeElement_Geometry3D(CX3DImporter_NodeElement::ENET_Sphere, NodeElement_Cur);
		if(!def.empty()) ne->ID = def;

		StandardShapes::MakeSphere(tess, tlist);
		// copy data from temp array and apply scale
		for(std::vector<aiVector3D>::iterator it = tlist.begin(); it != tlist.end(); ++it)
		{
			((CX3DImporter_NodeElement_Geometry3D*)ne)->Vertices.push_back(*it * radius);
		}

		((CX3DImporter_NodeElement_Geometry3D*)ne)->Solid = solid;
		((CX3DImporter_NodeElement_Geometry3D*)ne)->NumIndices = 3;
		// check for X3DMetadataObject childs.
		if(!mReader->isEmptyElement())
			ParseNode_Metadata(ne, "Sphere");
		else
			NodeElement_Cur->Child.push_back(ne);// add made object as child to current element

		NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
	}// if(!use.empty()) else
}

}// namespace Assimp

#endif // !ASSIMP_BUILD_NO_X3D_IMPORTER
