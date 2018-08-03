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
/// \file   X3DImporter_Group.cpp
/// \brief  Parsing data from nodes of "Grouping" set of X3D.
/// \date   2015-2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_X3D_IMPORTER

#include "X3DImporter.hpp"
#include "X3DImporter_Macro.hpp"

namespace Assimp
{

// <Group
// DEF=""              ID
// USE=""              IDREF
// bboxCenter="0 0 0"  SFVec3f [initializeOnly]
// bboxSize="-1 -1 -1" SFVec3f [initializeOnly]
// >
//    <!-- ChildContentModel -->
// ChildContentModel is the child-node content model corresponding to X3DChildNode, combining all profiles. ChildContentModel can contain most nodes,
// other Grouping nodes, Prototype declarations and ProtoInstances in any order and any combination. When the assigned profile is less than Full, the
// precise palette of legal nodes that are available depends on assigned profile and components.
// A ProtoInstance node (with the proper node type) can be substituted for any node in this content model.
// </Group>
// A Group node contains children nodes without introducing a new transformation. It is equivalent to a Transform node containing an identity transform.
void X3DImporter::ParseNode_Grouping_Group()
{
    std::string def, use;

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		CX3DImporter_NodeElement* ne;

		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Group, ne);
	}
	else
	{
		ParseHelper_Group_Begin();// create new grouping element and go deeper if node has children.
		// at this place new group mode created and made current, so we can name it.
		if(!def.empty()) NodeElement_Cur->ID = def;
		// in grouping set of nodes check X3DMetadataObject is not needed, because it is done in <Scene> parser function.

		// for empty element exit from node in that place
		if(mReader->isEmptyElement()) ParseHelper_Node_Exit();
	}// if(!use.empty()) else
}

void X3DImporter::ParseNode_Grouping_GroupEnd()
{
	ParseHelper_Node_Exit();// go up in scene graph
}

// <StaticGroup
// DEF=""              ID
// USE=""              IDREF
// bboxCenter="0 0 0"  SFVec3f [initializeOnly]
// bboxSize="-1 -1 -1" SFVec3f [initializeOnly]
// >
//    <!-- ChildContentModel -->
// ChildContentModel is the child-node content model corresponding to X3DChildNode, combining all profiles. ChildContentModel can contain most nodes,
// other Grouping nodes, Prototype declarations and ProtoInstances in any order and any combination. When the assigned profile is less than Full, the
// precise palette of legal nodes that are available depends on assigned profile and components.
// A ProtoInstance node (with the proper node type) can be substituted for any node in this content model.
// </StaticGroup>
// The StaticGroup node contains children nodes which cannot be modified. StaticGroup children are guaranteed to not change, send events, receive events or
// contain any USE references outside the StaticGroup.
void X3DImporter::ParseNode_Grouping_StaticGroup()
{
    std::string def, use;

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		CX3DImporter_NodeElement* ne;

		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Group, ne);
	}
	else
	{
		ParseHelper_Group_Begin(true);// create new grouping element and go deeper if node has children.
		// at this place new group mode created and made current, so we can name it.
		if(!def.empty()) NodeElement_Cur->ID = def;
		// in grouping set of nodes check X3DMetadataObject is not needed, because it is done in <Scene> parser function.

		// for empty element exit from node in that place
		if(mReader->isEmptyElement()) ParseHelper_Node_Exit();
	}// if(!use.empty()) else
}

void X3DImporter::ParseNode_Grouping_StaticGroupEnd()
{
	ParseHelper_Node_Exit();// go up in scene graph
}

// <Switch
// DEF=""              ID
// USE=""              IDREF
// bboxCenter="0 0 0"  SFVec3f [initializeOnly]
// bboxSize="-1 -1 -1" SFVec3f [initializeOnly]
// whichChoice="-1"    SFInt32 [inputOutput]
// >
//    <!-- ChildContentModel -->
// ChildContentModel is the child-node content model corresponding to X3DChildNode, combining all profiles. ChildContentModel can contain most nodes,
// other Grouping nodes, Prototype declarations and ProtoInstances in any order and any combination. When the assigned profile is less than Full, the
// precise palette of legal nodes that are available depends on assigned profile and components.
// A ProtoInstance node (with the proper node type) can be substituted for any node in this content model.
// </Switch>
// The Switch grouping node traverses zero or one of the nodes specified in the children field. The whichChoice field specifies the index of the child
// to traverse, with the first child having index 0. If whichChoice is less than zero or greater than the number of nodes in the children field, nothing
// is chosen.
void X3DImporter::ParseNode_Grouping_Switch()
{
    std::string def, use;
    int32_t whichChoice = -1;

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("whichChoice", whichChoice, XML_ReadNode_GetAttrVal_AsI32);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		CX3DImporter_NodeElement* ne;

		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Group, ne);
	}
	else
	{
		ParseHelper_Group_Begin();// create new grouping element and go deeper if node has children.
		// at this place new group mode created and made current, so we can name it.
		if(!def.empty()) NodeElement_Cur->ID = def;

		// also set values specific to this type of group
		((CX3DImporter_NodeElement_Group*)NodeElement_Cur)->UseChoice = true;
		((CX3DImporter_NodeElement_Group*)NodeElement_Cur)->Choice = whichChoice;
		// in grouping set of nodes check X3DMetadataObject is not needed, because it is done in <Scene> parser function.

		// for empty element exit from node in that place
		if(mReader->isEmptyElement()) ParseHelper_Node_Exit();
	}// if(!use.empty()) else
}

void X3DImporter::ParseNode_Grouping_SwitchEnd()
{
	// just exit from node. Defined choice will be accepted at postprocessing stage.
	ParseHelper_Node_Exit();// go up in scene graph
}

// <Transform
// DEF=""                     ID
// USE=""                     IDREF
// bboxCenter="0 0 0"         SFVec3f    [initializeOnly]
// bboxSize="-1 -1 -1"        SFVec3f    [initializeOnly]
// center="0 0 0"             SFVec3f    [inputOutput]
// rotation="0 0 1 0"         SFRotation [inputOutput]
// scale="1 1 1"              SFVec3f    [inputOutput]
// scaleOrientation="0 0 1 0" SFRotation [inputOutput]
// translation="0 0 0"        SFVec3f    [inputOutput]
// >
//    <!-- ChildContentModel -->
// ChildContentModel is the child-node content model corresponding to X3DChildNode, combining all profiles. ChildContentModel can contain most nodes,
// other Grouping nodes, Prototype declarations and ProtoInstances in any order and any combination. When the assigned profile is less than Full, the
// precise palette of legal nodes that are available depends on assigned profile and components.
// A ProtoInstance node (with the proper node type) can be substituted for any node in this content model.
// </Transform>
// The Transform node is a grouping node that defines a coordinate system for its children that is relative to the coordinate systems of its ancestors.
// Given a 3-dimensional point P and Transform node, P is transformed into point P' in its parent's coordinate system by a series of intermediate
// transformations. In matrix transformation notation, where C (center), SR (scaleOrientation), T (translation), R (rotation), and S (scale) are the
// equivalent transformation matrices,
//   P' = T * C * R * SR * S * -SR * -C * P
void X3DImporter::ParseNode_Grouping_Transform()
{
    aiVector3D center(0, 0, 0);
    float rotation[4] = {0, 0, 1, 0};
    aiVector3D scale(1, 1, 1);// A value of zero indicates that any child geometry shall not be displayed
    float scale_orientation[4] = {0, 0, 1, 0};
    aiVector3D translation(0, 0, 0);
    aiMatrix4x4 matr, tmatr;
    std::string use, def;

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_REF("center", center, XML_ReadNode_GetAttrVal_AsVec3f);
		MACRO_ATTRREAD_CHECK_REF("scale", scale, XML_ReadNode_GetAttrVal_AsVec3f);
		MACRO_ATTRREAD_CHECK_REF("translation", translation, XML_ReadNode_GetAttrVal_AsVec3f);
		if(an == "rotation")
		{
			std::vector<float> tvec;

			XML_ReadNode_GetAttrVal_AsArrF(idx, tvec);
			if(tvec.size() != 4) throw DeadlyImportError("<Transform>: rotation vector must have 4 elements.");

			memcpy(rotation, tvec.data(), sizeof(rotation));

			continue;
		}

		if(an == "scaleOrientation")
		{
			std::vector<float> tvec;
			XML_ReadNode_GetAttrVal_AsArrF(idx, tvec);
            if ( tvec.size() != 4 )
            {
                throw DeadlyImportError( "<Transform>: scaleOrientation vector must have 4 elements." );
            }

			::memcpy(scale_orientation, tvec.data(), sizeof(scale_orientation) );

			continue;
		}

	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		CX3DImporter_NodeElement* ne( nullptr );

		MACRO_USE_CHECKANDAPPLY(def, use, ENET_Group, ne);
	}
	else
	{
		ParseHelper_Group_Begin();// create new grouping element and go deeper if node has children.
		// at this place new group mode created and made current, so we can name it.
        if ( !def.empty() )
        {
            NodeElement_Cur->ID = def;
        }

		//
		// also set values specific to this type of group
		//
		// calculate transformation matrix
		aiMatrix4x4::Translation(translation, matr);// T
		aiMatrix4x4::Translation(center, tmatr);// C
		matr *= tmatr;
		aiMatrix4x4::Rotation(rotation[3], aiVector3D(rotation[0], rotation[1], rotation[2]), tmatr);// R
		matr *= tmatr;
		aiMatrix4x4::Rotation(scale_orientation[3], aiVector3D(scale_orientation[0], scale_orientation[1], scale_orientation[2]), tmatr);// SR
		matr *= tmatr;
		aiMatrix4x4::Scaling(scale, tmatr);// S
		matr *= tmatr;
		aiMatrix4x4::Rotation(-scale_orientation[3], aiVector3D(scale_orientation[0], scale_orientation[1], scale_orientation[2]), tmatr);// -SR
		matr *= tmatr;
		aiMatrix4x4::Translation(-center, tmatr);// -C
		matr *= tmatr;
		// and assign it
		((CX3DImporter_NodeElement_Group*)NodeElement_Cur)->Transformation = matr;
		// in grouping set of nodes check X3DMetadataObject is not needed, because it is done in <Scene> parser function.

		// for empty element exit from node in that place
        if ( mReader->isEmptyElement() )
        {
            ParseHelper_Node_Exit();
        }
	}// if(!use.empty()) else
}

void X3DImporter::ParseNode_Grouping_TransformEnd()
{
	ParseHelper_Node_Exit();// go up in scene graph
}

}// namespace Assimp

#endif // !ASSIMP_BUILD_NO_X3D_IMPORTER
