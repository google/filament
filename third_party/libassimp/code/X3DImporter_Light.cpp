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
/// \file   X3DImporter_Light.cpp
/// \brief  Parsing data from nodes of "Lighting" set of X3D.
/// \date   2015-2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_X3D_IMPORTER

#include "X3DImporter.hpp"
#include "X3DImporter_Macro.hpp"
#include "StringUtils.h"

namespace Assimp {

// <DirectionalLight
// DEF=""               ID
// USE=""               IDREF
// ambientIntensity="0" SFFloat [inputOutput]
// color="1 1 1"        SFColor [inputOutput]
// direction="0 0 -1"   SFVec3f [inputOutput]
// global="false"       SFBool  [inputOutput]
// intensity="1"        SFFloat [inputOutput]
// on="true"            SFBool  [inputOutput]
// />
void X3DImporter::ParseNode_Lighting_DirectionalLight()
{
    std::string def, use;
    float ambientIntensity = 0;
    aiColor3D color(1, 1, 1);
    aiVector3D direction(0, 0, -1);
    bool global = false;
    float intensity = 1;
    bool on = true;
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("ambientIntensity", ambientIntensity, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_REF("color", color, XML_ReadNode_GetAttrVal_AsCol3f);
		MACRO_ATTRREAD_CHECK_REF("direction", direction, XML_ReadNode_GetAttrVal_AsVec3f);
		MACRO_ATTRREAD_CHECK_RET("global", global, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("intensity", intensity, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_RET("on", on, XML_ReadNode_GetAttrVal_AsBool);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_DirectionalLight, ne);
	}
	else
	{
		if(on)
		{
			// create and if needed - define new geometry object.
			ne = new CX3DImporter_NodeElement_Light(CX3DImporter_NodeElement::ENET_DirectionalLight, NodeElement_Cur);
			if(!def.empty())
				ne->ID = def;
			else
				ne->ID = "DirectionalLight_" + to_string((size_t)ne);// make random name

			((CX3DImporter_NodeElement_Light*)ne)->AmbientIntensity = ambientIntensity;
			((CX3DImporter_NodeElement_Light*)ne)->Color = color;
			((CX3DImporter_NodeElement_Light*)ne)->Direction = direction;
			((CX3DImporter_NodeElement_Light*)ne)->Global = global;
			((CX3DImporter_NodeElement_Light*)ne)->Intensity = intensity;
			// Assimp want a node with name similar to a light. "Why? I don't no." )
			ParseHelper_Group_Begin(false);

			NodeElement_Cur->ID = ne->ID;// assign name to node and return to light element.
			ParseHelper_Node_Exit();
			// check for child nodes
			if(!mReader->isEmptyElement())
				ParseNode_Metadata(ne, "DirectionalLight");
			else
				NodeElement_Cur->Child.push_back(ne);// add made object as child to current element

			NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
		}// if(on)
	}// if(!use.empty()) else
}

// <PointLight
// DEF=""               ID
// USE=""               IDREF
// ambientIntensity="0" SFFloat [inputOutput]
// attenuation="1 0 0"  SFVec3f [inputOutput]
// color="1 1 1"        SFColor [inputOutput]
// global="true"        SFBool  [inputOutput]
// intensity="1"        SFFloat [inputOutput]
// location="0 0 0"     SFVec3f [inputOutput]
// on="true"            SFBool  [inputOutput]
// radius="100"         SFFloat [inputOutput]
// />
void X3DImporter::ParseNode_Lighting_PointLight()
{
    std::string def, use;
    float ambientIntensity = 0;
    aiVector3D attenuation( 1, 0, 0 );
    aiColor3D color( 1, 1, 1 );
    bool global = true;
    float intensity = 1;
    aiVector3D location( 0, 0, 0 );
    bool on = true;
    float radius = 100;
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("ambientIntensity", ambientIntensity, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_REF("attenuation", attenuation, XML_ReadNode_GetAttrVal_AsVec3f);
		MACRO_ATTRREAD_CHECK_REF("color", color, XML_ReadNode_GetAttrVal_AsCol3f);
		MACRO_ATTRREAD_CHECK_RET("global", global, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("intensity", intensity, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_REF("location", location, XML_ReadNode_GetAttrVal_AsVec3f);
		MACRO_ATTRREAD_CHECK_RET("on", on, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("radius", radius, XML_ReadNode_GetAttrVal_AsFloat);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_PointLight, ne);
	}
	else
	{
		if(on)
		{
			// create and if needed - define new geometry object.
			ne = new CX3DImporter_NodeElement_Light(CX3DImporter_NodeElement::ENET_PointLight, NodeElement_Cur);
			if(!def.empty()) ne->ID = def;

			((CX3DImporter_NodeElement_Light*)ne)->AmbientIntensity = ambientIntensity;
			((CX3DImporter_NodeElement_Light*)ne)->Attenuation = attenuation;
			((CX3DImporter_NodeElement_Light*)ne)->Color = color;
			((CX3DImporter_NodeElement_Light*)ne)->Global = global;
			((CX3DImporter_NodeElement_Light*)ne)->Intensity = intensity;
			((CX3DImporter_NodeElement_Light*)ne)->Location = location;
			((CX3DImporter_NodeElement_Light*)ne)->Radius = radius;
			// Assimp want a node with name similar to a light. "Why? I don't no." )
			ParseHelper_Group_Begin(false);
			// make random name
			if(ne->ID.empty()) ne->ID = "PointLight_" + to_string((size_t)ne);

			NodeElement_Cur->ID = ne->ID;// assign name to node and return to light element.
			ParseHelper_Node_Exit();
			// check for child nodes
			if(!mReader->isEmptyElement())
				ParseNode_Metadata(ne, "PointLight");
			else
				NodeElement_Cur->Child.push_back(ne);// add made object as child to current element

			NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
		}// if(on)
	}// if(!use.empty()) else
}

// <SpotLight
// DEF=""                 ID
// USE=""                 IDREF
// ambientIntensity="0"   SFFloat [inputOutput]
// attenuation="1 0 0"    SFVec3f [inputOutput]
// beamWidth="0.7854"     SFFloat [inputOutput]
// color="1 1 1"          SFColor [inputOutput]
// cutOffAngle="1.570796" SFFloat [inputOutput]
// direction="0 0 -1"     SFVec3f [inputOutput]
// global="true"          SFBool  [inputOutput]
// intensity="1"          SFFloat [inputOutput]
// location="0 0 0"       SFVec3f [inputOutput]
// on="true"              SFBool  [inputOutput]
// radius="100"           SFFloat [inputOutput]
// />
void X3DImporter::ParseNode_Lighting_SpotLight()
{
    std::string def, use;
    float ambientIntensity = 0;
    aiVector3D attenuation( 1, 0, 0 );
    float beamWidth = 0.7854f;
    aiColor3D color( 1, 1, 1 );
    float cutOffAngle = 1.570796f;
    aiVector3D direction( 0, 0, -1 );
    bool global = true;
    float intensity = 1;
    aiVector3D location( 0, 0, 0 );
    bool on = true;
    float radius = 100;
    CX3DImporter_NodeElement* ne( nullptr );

	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECKUSEDEF_RET(def, use);
		MACRO_ATTRREAD_CHECK_RET("ambientIntensity", ambientIntensity, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_REF("attenuation", attenuation, XML_ReadNode_GetAttrVal_AsVec3f);
		MACRO_ATTRREAD_CHECK_RET("beamWidth", beamWidth, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_REF("color", color, XML_ReadNode_GetAttrVal_AsCol3f);
		MACRO_ATTRREAD_CHECK_RET("cutOffAngle", cutOffAngle, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_REF("direction", direction, XML_ReadNode_GetAttrVal_AsVec3f);
		MACRO_ATTRREAD_CHECK_RET("global", global, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("intensity", intensity, XML_ReadNode_GetAttrVal_AsFloat);
		MACRO_ATTRREAD_CHECK_REF("location", location, XML_ReadNode_GetAttrVal_AsVec3f);
		MACRO_ATTRREAD_CHECK_RET("on", on, XML_ReadNode_GetAttrVal_AsBool);
		MACRO_ATTRREAD_CHECK_RET("radius", radius, XML_ReadNode_GetAttrVal_AsFloat);
	MACRO_ATTRREAD_LOOPEND;

	// if "USE" defined then find already defined element.
	if(!use.empty())
	{
		MACRO_USE_CHECKANDAPPLY(def, use, ENET_SpotLight, ne);
	}
	else
	{
		if(on)
		{
			// create and if needed - define new geometry object.
			ne = new CX3DImporter_NodeElement_Light(CX3DImporter_NodeElement::ENET_SpotLight, NodeElement_Cur);
			if(!def.empty()) ne->ID = def;

			if(beamWidth > cutOffAngle) beamWidth = cutOffAngle;

			((CX3DImporter_NodeElement_Light*)ne)->AmbientIntensity = ambientIntensity;
			((CX3DImporter_NodeElement_Light*)ne)->Attenuation = attenuation;
			((CX3DImporter_NodeElement_Light*)ne)->BeamWidth = beamWidth;
			((CX3DImporter_NodeElement_Light*)ne)->Color = color;
			((CX3DImporter_NodeElement_Light*)ne)->CutOffAngle = cutOffAngle;
			((CX3DImporter_NodeElement_Light*)ne)->Direction = direction;
			((CX3DImporter_NodeElement_Light*)ne)->Global = global;
			((CX3DImporter_NodeElement_Light*)ne)->Intensity = intensity;
			((CX3DImporter_NodeElement_Light*)ne)->Location = location;
			((CX3DImporter_NodeElement_Light*)ne)->Radius = radius;

			// Assimp want a node with name similar to a light. "Why? I don't no." )
			ParseHelper_Group_Begin(false);
			// make random name
			if(ne->ID.empty()) ne->ID = "SpotLight_" + to_string((size_t)ne);

			NodeElement_Cur->ID = ne->ID;// assign name to node and return to light element.
			ParseHelper_Node_Exit();
			// check for child nodes
			if(!mReader->isEmptyElement())
				ParseNode_Metadata(ne, "SpotLight");
			else
				NodeElement_Cur->Child.push_back(ne);// add made object as child to current element

			NodeElement_List.push_back(ne);// add element to node element list because its a new object in graph
		}// if(on)
	}// if(!use.empty()) else
}

}// namespace Assimp

#endif // !ASSIMP_BUILD_NO_X3D_IMPORTER
