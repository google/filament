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
/// \file   X3DImporter_Shape.cpp
/// \brief  Parsing data from nodes of "Shape" set of X3D.
/// \date   2015-2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_X3D_IMPORTER

#include "X3DImporter.hpp"
#include "X3DImporter_Macro.hpp"

namespace Assimp
{

// <Shape
// DEF=""              ID
// USE=""              IDREF
// bboxCenter="0 0 0"  SFVec3f [initializeOnly]
// bboxSize="-1 -1 -1" SFVec3f [initializeOnly]
// >
//    <!-- ShapeChildContentModel -->
// "ShapeChildContentModel is the child-node content model corresponding to X3DShapeNode. ShapeChildContentModel can contain a single Appearance node and a
// single geometry node, in any order.
// A ProtoInstance node (with the proper node type) can be substituted for any node in this content model."
// </Shape>
// A Shape node is unlit if either of the following is true:
//     The shape's appearance field is NULL (default).
//     The material field in the Appearance node is NULL (default).
// NOTE Geometry nodes that represent lines or points do not support lighting.
void X3DImporter::ParseNode_Shape_Shape()
{
    std::string use, def;
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Shape, ne);
	}
	else
	{
		// create and if needed - define new geometry object.
		ne = new CX3DImporter_NodeElement_Shape(NodeElement_Cur);
		if(!def.empty()) ne->ID = def;

        // check for child nodes
        if(!mReader->isEmptyElement())
        {
			ParseHelper_Node_Enter(ne);
			MACRO_NODECHECK_LOOPBEGIN("Shape");
				// check for appearance node
				if(XML_CheckNode_NameEqual("Appearance")) { ParseNode_Shape_Appearance(); continue; }
				// check for X3DGeometryNodes
				if(XML_CheckNode_NameEqual("Arc2D")) { ParseNode_Geometry2D_Arc2D(); continue; }
				if(XML_CheckNode_NameEqual("ArcClose2D")) { ParseNode_Geometry2D_ArcClose2D(); continue; }
				if(XML_CheckNode_NameEqual("Circle2D")) { ParseNode_Geometry2D_Circle2D(); continue; }
				if(XML_CheckNode_NameEqual("Disk2D")) { ParseNode_Geometry2D_Disk2D(); continue; }
				if(XML_CheckNode_NameEqual("Polyline2D")) { ParseNode_Geometry2D_Polyline2D(); continue; }
				if(XML_CheckNode_NameEqual("Polypoint2D")) { ParseNode_Geometry2D_Polypoint2D(); continue; }
				if(XML_CheckNode_NameEqual("Rectangle2D")) { ParseNode_Geometry2D_Rectangle2D(); continue; }
				if(XML_CheckNode_NameEqual("TriangleSet2D")) { ParseNode_Geometry2D_TriangleSet2D(); continue; }
				if(XML_CheckNode_NameEqual("Box")) { ParseNode_Geometry3D_Box(); continue; }
				if(XML_CheckNode_NameEqual("Cone")) { ParseNode_Geometry3D_Cone(); continue; }
				if(XML_CheckNode_NameEqual("Cylinder")) { ParseNode_Geometry3D_Cylinder(); continue; }
				if(XML_CheckNode_NameEqual("ElevationGrid")) { ParseNode_Geometry3D_ElevationGrid(); continue; }
				if(XML_CheckNode_NameEqual("Extrusion")) { ParseNode_Geometry3D_Extrusion(); continue; }
				if(XML_CheckNode_NameEqual("IndexedFaceSet")) { ParseNode_Geometry3D_IndexedFaceSet(); continue; }
				if(XML_CheckNode_NameEqual("Sphere")) { ParseNode_Geometry3D_Sphere(); continue; }
				if(XML_CheckNode_NameEqual("IndexedLineSet")) { ParseNode_Rendering_IndexedLineSet(); continue; }
				if(XML_CheckNode_NameEqual("LineSet")) { ParseNode_Rendering_LineSet(); continue; }
				if(XML_CheckNode_NameEqual("PointSet")) { ParseNode_Rendering_PointSet(); continue; }
				if(XML_CheckNode_NameEqual("IndexedTriangleFanSet")) { ParseNode_Rendering_IndexedTriangleFanSet(); continue; }
				if(XML_CheckNode_NameEqual("IndexedTriangleSet")) { ParseNode_Rendering_IndexedTriangleSet(); continue; }
				if(XML_CheckNode_NameEqual("IndexedTriangleStripSet")) { ParseNode_Rendering_IndexedTriangleStripSet(); continue; }
				if(XML_CheckNode_NameEqual("TriangleFanSet")) { ParseNode_Rendering_TriangleFanSet(); continue; }
				if(XML_CheckNode_NameEqual("TriangleSet")) { ParseNode_Rendering_TriangleSet(); continue; }
				if(XML_CheckNode_NameEqual("TriangleStripSet")) { ParseNode_Rendering_TriangleStripSet(); continue; }
				// check for X3DMetadataObject
				if(!ParseHelper_CheckRead_X3DMetadataObject()) XML_CheckNode_SkipUnsupported("Shape");

			MACRO_NODECHECK_LOOPEND("Shape");
			ParseHelper_Node_Exit();
		}// if(!mReader->isEmptyElement())
		else
		{
			NodeElement_Cur->Child.push_back(ne);// add made object as child to current element
		}

		NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
	}// if(!use.empty()) else
}

// <Appearance
// DEF="" ID
// USE="" IDREF
// >
// <!-- AppearanceChildContentModel -->
// "Child-node content model corresponding to X3DAppearanceChildNode. Appearance can contain FillProperties, LineProperties, Material, any Texture node and
// any TextureTransform node, in any order. No more than one instance of these nodes is allowed. Appearance may also contain multiple shaders (ComposedShader,
// PackagedShader, ProgramShader).
// A ProtoInstance node (with the proper node type) can be substituted for any node in this content model."
// </Appearance>
void X3DImporter::ParseNode_Shape_Appearance()
{
    std::string use, def;
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Appearance, ne);
	}
	else
	{
		// create and if needed - define new geometry object.
		ne = new CX3DImporter_NodeElement_Appearance(NodeElement_Cur);
		if(!def.empty()) ne->ID = def;

        // check for child nodes
        if(!mReader->isEmptyElement())
        {
			ParseHelper_Node_Enter(ne);
			MACRO_NODECHECK_LOOPBEGIN("Appearance");
				if(XML_CheckNode_NameEqual("Material")) { ParseNode_Shape_Material(); continue; }
				if(XML_CheckNode_NameEqual("ImageTexture")) { ParseNode_Texturing_ImageTexture(); continue; }
				if(XML_CheckNode_NameEqual("TextureTransform")) { ParseNode_Texturing_TextureTransform(); continue; }
				// check for X3DMetadataObject
				if(!ParseHelper_CheckRead_X3DMetadataObject()) XML_CheckNode_SkipUnsupported("Appearance");

			MACRO_NODECHECK_LOOPEND("Appearance");
			ParseHelper_Node_Exit();
		}// if(!mReader->isEmptyElement())
		else
		{
			NodeElement_Cur->Child.push_back(ne);// add made object as child to current element
		}

		NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
	}// if(!use.empty()) else
}

// <Material
// DEF=""                     ID
// USE=""                     IDREF
// ambientIntensity="0.2"     SFFloat [inputOutput]
// diffuseColor="0.8 0.8 0.8" SFColor [inputOutput]
// emissiveColor="0 0 0"      SFColor [inputOutput]
// shininess="0.2"            SFFloat [inputOutput]
// specularColor="0 0 0"      SFColor [inputOutput]
// transparency="0"           SFFloat [inputOutput]
// />
void X3DImporter::ParseNode_Shape_Material()
{
    std::string use, def;
    float ambientIntensity = 0.2f;
    float shininess = 0.2f;
    float transparency = 0;
    aiColor3D diffuseColor(0.8f, 0.8f, 0.8f);
    aiColor3D emissiveColor(0, 0, 0);
    aiColor3D specularColor(0, 0, 0);
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("ambientIntensity", ambientIntensity, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_RET("shininess", shininess, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_RET("transparency", transparency, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_REF("diffuseColor", diffuseColor, XML_ReadNode_GetAttrVal_AsCol3f);
		MACRO_ATTRREAD_CHECK_REF("emissiveColor", emissiveColor, XML_ReadNode_GetAttrVal_AsCol3f);
		MACRO_ATTRREAD_CHECK_REF("specularColor", specularColor, XML_ReadNode_GetAttrVal_AsCol3f);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Material, ne);
	}
	else
	{
		// create and if needed - define new geometry object.
		ne = new CX3DImporter_NodeElement_Material(NodeElement_Cur);
		if(!def.empty()) ne->ID = def;

		((CX3DImporter_NodeElement_Material*)ne)->AmbientIntensity = ambientIntensity;
		((CX3DImporter_NodeElement_Material*)ne)->Shininess = shininess;
		((CX3DImporter_NodeElement_Material*)ne)->Transparency = transparency;
		((CX3DImporter_NodeElement_Material*)ne)->DiffuseColor = diffuseColor;
		((CX3DImporter_NodeElement_Material*)ne)->EmissiveColor = emissiveColor;
		((CX3DImporter_NodeElement_Material*)ne)->SpecularColor = specularColor;
        // check for child nodes
		if(!mReader->isEmptyElement())
			ParseNode_Metadata(ne, "Material");
		else
			NodeElement_Cur->Child.push_back(ne);// add made object as child to current element

		NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
	}// if(!use.empty()) else
}

}// namespace Assimp

#endif // !ASSIMP_BUILD_NO_X3D_IMPORTER
