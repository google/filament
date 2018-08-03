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
/// \file   X3DImporter.cpp
/// \brief  X3D-format files importer for Assimp: main algorithm implementation.
/// \date   2015-2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_X3D_IMPORTER

#include "X3DImporter.hpp"
#include "X3DImporter_Macro.hpp"
#include "StringUtils.h"

// Header files, Assimp.
#include <assimp/DefaultIOSystem.h>
#include "fast_atof.h"
#include "FIReader.hpp"

// Header files, stdlib.
#include <memory>
#include <string>
#include <iterator>

namespace Assimp {

/// \var aiImporterDesc X3DImporter::Description
/// Constant which holds the importer description
const aiImporterDesc X3DImporter::Description = {
	"Extensible 3D(X3D) Importer",
	"smalcom",
	"",
	"See documentation in source code. Chapter: Limitations.",
	aiImporterFlags_SupportTextFlavour | aiImporterFlags_SupportBinaryFlavour | aiImporterFlags_LimitedSupport | aiImporterFlags_Experimental,
	0,
	0,
	0,
	0,
	"x3d x3db"
};

//const std::regex X3DImporter::pattern_nws(R"([^, \t\r\n]+)");
//const std::regex X3DImporter::pattern_true(R"(^\s*(?:true|1)\s*$)", std::regex::icase);

struct WordIterator: public std::iterator<std::input_iterator_tag, const char*> {
    static const char *whitespace;
    const char *start_, *end_;
    WordIterator(const char *start, const char *end): start_(start), end_(end) {
        start_ = start + strspn(start, whitespace);
        if (start_ >= end_) {
            start_ = 0;
        }
    }
    WordIterator(): start_(0), end_(0) {}
    WordIterator(const WordIterator &other): start_(other.start_), end_(other.end_) {}
    WordIterator &operator=(const WordIterator &other) {
        start_ = other.start_;
        end_ = other.end_;
        return *this;
    }
    bool operator==(const WordIterator &other) const { return start_ == other.start_; }
    bool operator!=(const WordIterator &other) const { return start_ != other.start_; }
    WordIterator &operator++() {
        start_ += strcspn(start_, whitespace);
        start_ += strspn(start_, whitespace);
        if (start_ >= end_) {
            start_ = 0;
        }
        return *this;
    }
    WordIterator operator++(int) {
        WordIterator result(*this);
        ++(*this);
        return result;
    }
    const char *operator*() const { return start_; }
};

const char *WordIterator::whitespace = ", \t\r\n";

X3DImporter::X3DImporter()
: NodeElement_Cur( nullptr )
, mReader( nullptr ) {
    // empty
}

X3DImporter::~X3DImporter() {
    // Clear() is accounting if data already is deleted. So, just check again if all data is deleted.
    Clear();
}

void X3DImporter::Clear() {
	NodeElement_Cur = nullptr;
	// Delete all elements
	if(NodeElement_List.size()) {
        for ( std::list<CX3DImporter_NodeElement*>::iterator it = NodeElement_List.begin(); it != NodeElement_List.end(); it++ ) {
            delete *it;
        }
		NodeElement_List.clear();
	}
}


/*********************************************************************************************************************************************/
/************************************************************ Functions: find set ************************************************************/
/*********************************************************************************************************************************************/

bool X3DImporter::FindNodeElement_FromRoot(const std::string& pID, const CX3DImporter_NodeElement::EType pType, CX3DImporter_NodeElement** pElement)
{
	for(std::list<CX3DImporter_NodeElement*>::iterator it = NodeElement_List.begin(); it != NodeElement_List.end(); it++)
	{
		if(((*it)->Type == pType) && ((*it)->ID == pID))
		{
			if(pElement != nullptr) *pElement = *it;

			return true;
		}
	}// for(std::list<CX3DImporter_NodeElement*>::iterator it = NodeElement_List.begin(); it != NodeElement_List.end(); it++)

	return false;
}

bool X3DImporter::FindNodeElement_FromNode(CX3DImporter_NodeElement* pStartNode, const std::string& pID,
													const CX3DImporter_NodeElement::EType pType, CX3DImporter_NodeElement** pElement)
{
    bool found = false;// flag: true - if requested element is found.

	// Check if pStartNode - this is the element, we are looking for.
	if((pStartNode->Type == pType) && (pStartNode->ID == pID))
	{
		found = true;
        if ( pElement != nullptr )
        {
            *pElement = pStartNode;
        }

		goto fne_fn_end;
	}// if((pStartNode->Type() == pType) && (pStartNode->ID() == pID))

	// Check childs of pStartNode.
	for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = pStartNode->Child.begin(); ch_it != pStartNode->Child.end(); ch_it++)
	{
		found = FindNodeElement_FromNode(*ch_it, pID, pType, pElement);
        if ( found )
        {
            break;
        }
	}// for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = it->Child.begin(); ch_it != it->Child.end(); ch_it++)

fne_fn_end:

	return found;
}

bool X3DImporter::FindNodeElement(const std::string& pID, const CX3DImporter_NodeElement::EType pType, CX3DImporter_NodeElement** pElement)
{
    CX3DImporter_NodeElement* tnd = NodeElement_Cur;// temporary pointer to node.
    bool static_search = false;// flag: true if searching in static node.

    // At first check if we have deal with static node. Go up through parent nodes and check flag.
    while(tnd != nullptr)
    {
		if(tnd->Type == CX3DImporter_NodeElement::ENET_Group)
		{
			if(((CX3DImporter_NodeElement_Group*)tnd)->Static)
			{
				static_search = true;// Flag found, stop walking up. Node with static flag will holded in tnd variable.
				break;
			}
		}

		tnd = tnd->Parent;// go up in graph.
    }// while(tnd != nullptr)

    // at now call appropriate search function.
    if ( static_search )
    {
        return FindNodeElement_FromNode( tnd, pID, pType, pElement );
    }
    else
    {
        return FindNodeElement_FromRoot( pID, pType, pElement );
    }
}

/*********************************************************************************************************************************************/
/************************************************************ Functions: throw set ***********************************************************/
/*********************************************************************************************************************************************/

void X3DImporter::Throw_ArgOutOfRange(const std::string& pArgument)
{
	throw DeadlyImportError("Argument value is out of range for: \"" + pArgument + "\".");
}

void X3DImporter::Throw_CloseNotFound(const std::string& pNode)
{
	throw DeadlyImportError("Close tag for node <" + pNode + "> not found. Seems file is corrupt.");
}

void X3DImporter::Throw_ConvertFail_Str2ArrF(const std::string& pAttrValue)
{
	throw DeadlyImportError("In <" + std::string(mReader->getNodeName()) + "> failed to convert attribute value \"" + pAttrValue +
							"\" from string to array of floats.");
}

void X3DImporter::Throw_DEF_And_USE()
{
	throw DeadlyImportError("\"DEF\" and \"USE\" can not be defined both in <" + std::string(mReader->getNodeName()) + ">.");
}

void X3DImporter::Throw_IncorrectAttr(const std::string& pAttrName)
{
	throw DeadlyImportError("Node <" + std::string(mReader->getNodeName()) + "> has incorrect attribute \"" + pAttrName + "\".");
}

void X3DImporter::Throw_IncorrectAttrValue(const std::string& pAttrName)
{
	throw DeadlyImportError("Attribute \"" + pAttrName + "\" in node <" + std::string(mReader->getNodeName()) + "> has incorrect value.");
}

void X3DImporter::Throw_MoreThanOnceDefined(const std::string& pNodeType, const std::string& pDescription)
{
	throw DeadlyImportError("\"" + pNodeType + "\" node can be used only once in " + mReader->getNodeName() + ". Description: " + pDescription);
}

void X3DImporter::Throw_TagCountIncorrect(const std::string& pNode)
{
	throw DeadlyImportError("Count of open and close tags for node <" + pNode + "> are not equivalent. Seems file is corrupt.");
}

void X3DImporter::Throw_USE_NotFound(const std::string& pAttrValue)
{
	throw DeadlyImportError("Not found node with name \"" + pAttrValue + "\" in <" + std::string(mReader->getNodeName()) + ">.");
}

/*********************************************************************************************************************************************/
/************************************************************* Functions: XML set ************************************************************/
/*********************************************************************************************************************************************/

void X3DImporter::XML_CheckNode_MustBeEmpty()
{
	if(!mReader->isEmptyElement()) throw DeadlyImportError(std::string("Node <") + mReader->getNodeName() + "> must be empty.");
}

void X3DImporter::XML_CheckNode_SkipUnsupported(const std::string& pParentNodeName)
{
    static const size_t Uns_Skip_Len = 192;
    const char* Uns_Skip[ Uns_Skip_Len ] = {
	    // CAD geometry component
	    "CADAssembly", "CADFace", "CADLayer", "CADPart", "IndexedQuadSet", "QuadSet",
	    // Core
	    "ROUTE", "ExternProtoDeclare", "ProtoDeclare", "ProtoInstance", "ProtoInterface", "WorldInfo",
	    // Distributed interactive simulation (DIS) component
	    "DISEntityManager", "DISEntityTypeMapping", "EspduTransform", "ReceiverPdu", "SignalPdu", "TransmitterPdu",
	    // Cube map environmental texturing component
	    "ComposedCubeMapTexture", "GeneratedCubeMapTexture", "ImageCubeMapTexture",
	    // Environmental effects component
	    "Background", "Fog", "FogCoordinate", "LocalFog", "TextureBackground",
	    // Environmental sensor component
	    "ProximitySensor", "TransformSensor", "VisibilitySensor",
	    // Followers component
	    "ColorChaser", "ColorDamper", "CoordinateChaser", "CoordinateDamper", "OrientationChaser", "OrientationDamper", "PositionChaser", "PositionChaser2D",
	    "PositionDamper", "PositionDamper2D", "ScalarChaser", "ScalarDamper", "TexCoordChaser2D", "TexCoordDamper2D",
	    // Geospatial component
	    "GeoCoordinate", "GeoElevationGrid", "GeoLocation", "GeoLOD", "GeoMetadata", "GeoOrigin", "GeoPositionInterpolator", "GeoProximitySensor",
	    "GeoTouchSensor", "GeoTransform", "GeoViewpoint",
	    // Humanoid Animation (H-Anim) component
	    "HAnimDisplacer", "HAnimHumanoid", "HAnimJoint", "HAnimSegment", "HAnimSite",
	    // Interpolation component
	    "ColorInterpolator", "CoordinateInterpolator", "CoordinateInterpolator2D", "EaseInEaseOut", "NormalInterpolator", "OrientationInterpolator",
	    "PositionInterpolator", "PositionInterpolator2D", "ScalarInterpolator", "SplinePositionInterpolator", "SplinePositionInterpolator2D",
	    "SplineScalarInterpolator", "SquadOrientationInterpolator",
	    // Key device sensor component
	    "KeySensor", "StringSensor",
	    // Layering component
	    "Layer", "LayerSet", "Viewport",
	    // Layout component
	    "Layout", "LayoutGroup", "LayoutLayer", "ScreenFontStyle", "ScreenGroup",
	    // Navigation component
	    "Billboard", "Collision", "LOD", "NavigationInfo", "OrthoViewpoint", "Viewpoint", "ViewpointGroup",
	    // Networking component
	    "EXPORT", "IMPORT", "Anchor", "LoadSensor",
	    // NURBS component
	    "Contour2D", "ContourPolyline2D", "CoordinateDouble", "NurbsCurve", "NurbsCurve2D", "NurbsOrientationInterpolator", "NurbsPatchSurface",
	    "NurbsPositionInterpolator", "NurbsSet", "NurbsSurfaceInterpolator", "NurbsSweptSurface", "NurbsSwungSurface", "NurbsTextureCoordinate",
	    "NurbsTrimmedSurface",
	    // Particle systems component
	    "BoundedPhysicsModel", "ConeEmitter", "ExplosionEmitter", "ForcePhysicsModel", "ParticleSystem", "PointEmitter", "PolylineEmitter", "SurfaceEmitter",
	    "VolumeEmitter", "WindPhysicsModel",
	    // Picking component
	    "LinePickSensor", "PickableGroup", "PointPickSensor", "PrimitivePickSensor", "VolumePickSensor",
	    // Pointing device sensor component
	    "CylinderSensor", "PlaneSensor", "SphereSensor", "TouchSensor",
	    // Rendering component
	    "ClipPlane",
	    // Rigid body physics
	    "BallJoint", "CollidableOffset", "CollidableShape", "CollisionCollection", "CollisionSensor", "CollisionSpace", "Contact", "DoubleAxisHingeJoint",
	    "MotorJoint", "RigidBody", "RigidBodyCollection", "SingleAxisHingeJoint", "SliderJoint", "UniversalJoint",
	    // Scripting component
	    "Script",
	    // Programmable shaders component
	    "ComposedShader", "FloatVertexAttribute", "Matrix3VertexAttribute", "Matrix4VertexAttribute", "PackagedShader", "ProgramShader", "ShaderPart",
	    "ShaderProgram",
	    // Shape component
	    "FillProperties", "LineProperties", "TwoSidedMaterial",
	    // Sound component
	    "AudioClip", "Sound",
	    // Text component
	    "FontStyle", "Text",
	    // Texturing3D Component
	    "ComposedTexture3D", "ImageTexture3D", "PixelTexture3D", "TextureCoordinate3D", "TextureCoordinate4D", "TextureTransformMatrix3D", "TextureTransform3D",
	    // Texturing component
	    "MovieTexture", "MultiTexture", "MultiTextureCoordinate", "MultiTextureTransform", "PixelTexture", "TextureCoordinateGenerator", "TextureProperties",
	    // Time component
	    "TimeSensor",
	    // Event Utilities component
	    "BooleanFilter", "BooleanSequencer", "BooleanToggle", "BooleanTrigger", "IntegerSequencer", "IntegerTrigger", "TimeTrigger",
	    // Volume rendering component
	    "BlendedVolumeStyle", "BoundaryEnhancementVolumeStyle", "CartoonVolumeStyle", "ComposedVolumeStyle", "EdgeEnhancementVolumeStyle", "IsoSurfaceVolumeData",
	    "OpacityMapVolumeStyle", "ProjectionVolumeStyle", "SegmentedVolumeData", "ShadedVolumeStyle", "SilhouetteEnhancementVolumeStyle", "ToneMappedVolumeStyle",
	    "VolumeData"
    };

    const std::string nn( mReader->getNodeName() );
    bool found = false;
    bool close_found = false;

	for(size_t i = 0; i < Uns_Skip_Len; i++)
	{
		if(nn == Uns_Skip[i])
		{
			found = true;
			if(mReader->isEmptyElement())
			{
				close_found = true;

				goto casu_cres;
			}

			while(mReader->read())
			{
				if((mReader->getNodeType() == irr::io::EXN_ELEMENT_END) && (nn == mReader->getNodeName()))
				{
					close_found = true;

					goto casu_cres;
				}
			}
		}
	}

casu_cres:

	if(!found) throw DeadlyImportError("Unknown node \"" + nn + "\" in " + pParentNodeName + ".");

	if(close_found)
		LogInfo("Skipping node \"" + nn + "\" in " + pParentNodeName + ".");
	else
		Throw_CloseNotFound(nn);
}

bool X3DImporter::XML_SearchNode(const std::string& pNodeName)
{
	while(mReader->read())
	{
		if((mReader->getNodeType() == irr::io::EXN_ELEMENT) && XML_CheckNode_NameEqual(pNodeName)) return true;
	}

	return false;
}

bool X3DImporter::XML_ReadNode_GetAttrVal_AsBool(const int pAttrIdx)
{
    auto boolValue = std::dynamic_pointer_cast<const FIBoolValue>(mReader->getAttributeEncodedValue(pAttrIdx));
    if (boolValue) {
        if (boolValue->value.size() == 1) {
            return boolValue->value.front();
        }
        throw DeadlyImportError("Invalid bool value");
    }
    else {
        std::string val(mReader->getAttributeValue(pAttrIdx));

        if(val == "false")
            return false;
        else if(val == "true")
            return true;
        else
            throw DeadlyImportError("Bool attribute value can contain \"false\" or \"true\" not the \"" + val + "\"");
    }
}

float X3DImporter::XML_ReadNode_GetAttrVal_AsFloat(const int pAttrIdx)
{
    auto floatValue = std::dynamic_pointer_cast<const FIFloatValue>(mReader->getAttributeEncodedValue(pAttrIdx));
    if (floatValue) {
        if (floatValue->value.size() == 1) {
            return floatValue->value.front();
        }
        throw DeadlyImportError("Invalid float value");
    }
    else {
        std::string val;
        float tvalf;

        ParseHelper_FixTruncatedFloatString(mReader->getAttributeValue(pAttrIdx), val);
        fast_atoreal_move(val.c_str(), tvalf, false);

        return tvalf;
    }
}

int32_t X3DImporter::XML_ReadNode_GetAttrVal_AsI32(const int pAttrIdx)
{
    auto intValue = std::dynamic_pointer_cast<const FIIntValue>(mReader->getAttributeEncodedValue(pAttrIdx));
    if (intValue) {
        if (intValue->value.size() == 1) {
            return intValue->value.front();
        }
        throw DeadlyImportError("Invalid int value");
    }
    else {
        return strtol10(mReader->getAttributeValue(pAttrIdx));
    }
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsCol3f(const int pAttrIdx, aiColor3D& pValue)
{
    std::vector<float> tlist;
    std::vector<float>::iterator it;

	XML_ReadNode_GetAttrVal_AsArrF(pAttrIdx, tlist);
	if(tlist.size() != 3) Throw_ConvertFail_Str2ArrF(mReader->getAttributeValue(pAttrIdx));

	it = tlist.begin();
	pValue.r = *it++;
	pValue.g = *it++;
	pValue.b = *it;
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsVec2f(const int pAttrIdx, aiVector2D& pValue)
{
    std::vector<float> tlist;
    std::vector<float>::iterator it;

	XML_ReadNode_GetAttrVal_AsArrF(pAttrIdx, tlist);
	if(tlist.size() != 2) Throw_ConvertFail_Str2ArrF(mReader->getAttributeValue(pAttrIdx));

	it = tlist.begin();
	pValue.x = *it++;
	pValue.y = *it;
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsVec3f(const int pAttrIdx, aiVector3D& pValue)
{
    std::vector<float> tlist;
    std::vector<float>::iterator it;

	XML_ReadNode_GetAttrVal_AsArrF(pAttrIdx, tlist);
	if(tlist.size() != 3) Throw_ConvertFail_Str2ArrF(mReader->getAttributeValue(pAttrIdx));

	it = tlist.begin();
	pValue.x = *it++;
	pValue.y = *it++;
	pValue.z = *it;
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsArrB(const int pAttrIdx, std::vector<bool>& pValue)
{
    auto boolValue = std::dynamic_pointer_cast<const FIBoolValue>(mReader->getAttributeEncodedValue(pAttrIdx));
    if (boolValue) {
        pValue = boolValue->value;
    }
    else {
        const char *val = mReader->getAttributeValue(pAttrIdx);
        pValue.clear();

        //std::cregex_iterator wordItBegin(val, val + strlen(val), pattern_nws);
        //const std::cregex_iterator wordItEnd;
        //std::transform(wordItBegin, wordItEnd, std::back_inserter(pValue), [](const std::cmatch &match) { return std::regex_match(match.str(), pattern_true); });

        WordIterator wordItBegin(val, val + strlen(val));
        WordIterator wordItEnd;
        std::transform(wordItBegin, wordItEnd, std::back_inserter(pValue), [](const char *match) { return (::tolower(match[0]) == 't') || (match[0] == '1'); });
    }
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsArrI32(const int pAttrIdx, std::vector<int32_t>& pValue)
{
    auto intValue = std::dynamic_pointer_cast<const FIIntValue>(mReader->getAttributeEncodedValue(pAttrIdx));
    if (intValue) {
        pValue = intValue->value;
    }
    else {
        const char *val = mReader->getAttributeValue(pAttrIdx);
        pValue.clear();

        //std::cregex_iterator wordItBegin(val, val + strlen(val), pattern_nws);
        //const std::cregex_iterator wordItEnd;
        //std::transform(wordItBegin, wordItEnd, std::back_inserter(pValue), [](const std::cmatch &match) { return std::stoi(match.str()); });

        WordIterator wordItBegin(val, val + strlen(val));
        WordIterator wordItEnd;
        std::transform(wordItBegin, wordItEnd, std::back_inserter(pValue), [](const char *match) { return atoi(match); });
    }
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsArrF(const int pAttrIdx, std::vector<float>& pValue)
{
    auto floatValue = std::dynamic_pointer_cast<const FIFloatValue>(mReader->getAttributeEncodedValue(pAttrIdx));
    if (floatValue) {
        pValue = floatValue->value;
    }
    else {
        const char *val = mReader->getAttributeValue(pAttrIdx);
        pValue.clear();

        //std::cregex_iterator wordItBegin(val, val + strlen(val), pattern_nws);
        //const std::cregex_iterator wordItEnd;
        //std::transform(wordItBegin, wordItEnd, std::back_inserter(pValue), [](const std::cmatch &match) { return std::stof(match.str()); });

        WordIterator wordItBegin(val, val + strlen(val));
        WordIterator wordItEnd;
        std::transform(wordItBegin, wordItEnd, std::back_inserter(pValue), [](const char *match) { return static_cast<float>(atof(match)); });
    }
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsArrD(const int pAttrIdx, std::vector<double>& pValue)
{
    auto doubleValue = std::dynamic_pointer_cast<const FIDoubleValue>(mReader->getAttributeEncodedValue(pAttrIdx));
    if (doubleValue) {
        pValue = doubleValue->value;
    }
    else {
        const char *val = mReader->getAttributeValue(pAttrIdx);
        pValue.clear();

        //std::cregex_iterator wordItBegin(val, val + strlen(val), pattern_nws);
        //const std::cregex_iterator wordItEnd;
        //std::transform(wordItBegin, wordItEnd, std::back_inserter(pValue), [](const std::cmatch &match) { return std::stod(match.str()); });

        WordIterator wordItBegin(val, val + strlen(val));
        WordIterator wordItEnd;
        std::transform(wordItBegin, wordItEnd, std::back_inserter(pValue), [](const char *match) { return atof(match); });
    }
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsListCol3f(const int pAttrIdx, std::list<aiColor3D>& pValue)
{
    std::vector<float> tlist;

	XML_ReadNode_GetAttrVal_AsArrF(pAttrIdx, tlist);// read as list
	if(tlist.size() % 3) Throw_ConvertFail_Str2ArrF(mReader->getAttributeValue(pAttrIdx));

	// copy data to array
	for(std::vector<float>::iterator it = tlist.begin(); it != tlist.end();)
	{
		aiColor3D tcol;

		tcol.r = *it++;
		tcol.g = *it++;
		tcol.b = *it++;
		pValue.push_back(tcol);
	}
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsArrCol3f(const int pAttrIdx, std::vector<aiColor3D>& pValue)
{
    std::list<aiColor3D> tlist;

	XML_ReadNode_GetAttrVal_AsListCol3f(pAttrIdx, tlist);// read as list
	// and copy to array
	if(tlist.size() > 0)
	{
		pValue.reserve(tlist.size());
		for(std::list<aiColor3D>::iterator it = tlist.begin(); it != tlist.end(); it++) pValue.push_back(*it);
	}
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsListCol4f(const int pAttrIdx, std::list<aiColor4D>& pValue)
{
    std::vector<float> tlist;

	XML_ReadNode_GetAttrVal_AsArrF(pAttrIdx, tlist);// read as list
	if(tlist.size() % 4) Throw_ConvertFail_Str2ArrF(mReader->getAttributeValue(pAttrIdx));

	// copy data to array
	for(std::vector<float>::iterator it = tlist.begin(); it != tlist.end();)
	{
		aiColor4D tcol;

		tcol.r = *it++;
		tcol.g = *it++;
		tcol.b = *it++;
		tcol.a = *it++;
		pValue.push_back(tcol);
	}
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsArrCol4f(const int pAttrIdx, std::vector<aiColor4D>& pValue)
{
    std::list<aiColor4D> tlist;

	XML_ReadNode_GetAttrVal_AsListCol4f(pAttrIdx, tlist);// read as list
	// and copy to array
	if(tlist.size() > 0)
	{
		pValue.reserve(tlist.size());
        for ( std::list<aiColor4D>::iterator it = tlist.begin(); it != tlist.end(); it++ )
        {
            pValue.push_back( *it );
        }
	}
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsListVec2f(const int pAttrIdx, std::list<aiVector2D>& pValue)
{
    std::vector<float> tlist;

	XML_ReadNode_GetAttrVal_AsArrF(pAttrIdx, tlist);// read as list
    if ( tlist.size() % 2 )
    {
        Throw_ConvertFail_Str2ArrF( mReader->getAttributeValue( pAttrIdx ) );
    }

	// copy data to array
	for(std::vector<float>::iterator it = tlist.begin(); it != tlist.end();)
	{
		aiVector2D tvec;

		tvec.x = *it++;
		tvec.y = *it++;
		pValue.push_back(tvec);
	}
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsArrVec2f(const int pAttrIdx, std::vector<aiVector2D>& pValue)
{
    std::list<aiVector2D> tlist;

	XML_ReadNode_GetAttrVal_AsListVec2f(pAttrIdx, tlist);// read as list
	// and copy to array
	if(tlist.size() > 0)
	{
		pValue.reserve(tlist.size());
        for ( std::list<aiVector2D>::iterator it = tlist.begin(); it != tlist.end(); it++ )
        {
            pValue.push_back( *it );
        }
	}
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsListVec3f(const int pAttrIdx, std::list<aiVector3D>& pValue)
{
    std::vector<float> tlist;

	XML_ReadNode_GetAttrVal_AsArrF(pAttrIdx, tlist);// read as list
    if ( tlist.size() % 3 )
    {
        Throw_ConvertFail_Str2ArrF( mReader->getAttributeValue( pAttrIdx ) );
    }

	// copy data to array
	for(std::vector<float>::iterator it = tlist.begin(); it != tlist.end();)
	{
		aiVector3D tvec;

		tvec.x = *it++;
		tvec.y = *it++;
		tvec.z = *it++;
		pValue.push_back(tvec);
	}
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsArrVec3f(const int pAttrIdx, std::vector<aiVector3D>& pValue)
{
    std::list<aiVector3D> tlist;

	XML_ReadNode_GetAttrVal_AsListVec3f(pAttrIdx, tlist);// read as list
	// and copy to array
	if(tlist.size() > 0)
	{
		pValue.reserve(tlist.size());
        for ( std::list<aiVector3D>::iterator it = tlist.begin(); it != tlist.end(); it++ )
        {
            pValue.push_back( *it );
        }
	}
}

void X3DImporter::XML_ReadNode_GetAttrVal_AsListS(const int pAttrIdx, std::list<std::string>& pValue)
{
	// make copy of attribute value - strings list.
	const size_t tok_str_len = strlen(mReader->getAttributeValue(pAttrIdx));
    if ( 0 == tok_str_len )
    {
        Throw_IncorrectAttrValue( mReader->getAttributeName( pAttrIdx ) );
    }

	// get pointer to begin of value.
    char *tok_str = const_cast<char*>(mReader->getAttributeValue(pAttrIdx));
    char *tok_str_end = tok_str + tok_str_len;
	// string list has following format: attr_name='"s1" "s2" "sn"'.
	do
	{
		char* tbeg;
		char* tend;
		size_t tlen;
		std::string tstr;

		// find begin of string(element of string list): "sn".
		tbeg = strstr(tok_str, "\"");
		if(tbeg == nullptr) Throw_IncorrectAttrValue(mReader->getAttributeName(pAttrIdx));

		tbeg++;// forward pointer from '\"' symbol to next after it.
		tok_str = tbeg;
		// find end of string(element of string list): "sn".
		tend = strstr(tok_str, "\"");
		if(tend == nullptr) Throw_IncorrectAttrValue(mReader->getAttributeName(pAttrIdx));

		tok_str = tend + 1;
		// create storage for new string
		tlen = tend - tbeg;
		tstr.resize(tlen);// reserve enough space and copy data
		memcpy((void*)tstr.data(), tbeg, tlen);// not strcpy because end of copied string from tok_str has no terminator.
		// and store string in output list.
		pValue.push_back(tstr);
	} while(tok_str < tok_str_end);
}

/*********************************************************************************************************************************************/
/****************************************************** Functions: geometry helper set  ******************************************************/
/*********************************************************************************************************************************************/

aiVector3D X3DImporter::GeometryHelper_Make_Point2D(const float pAngle, const float pRadius)
{
	return aiVector3D(pRadius * std::cos(pAngle), pRadius * std::sin(pAngle), 0);
}

void X3DImporter::GeometryHelper_Make_Arc2D(const float pStartAngle, const float pEndAngle, const float pRadius, size_t pNumSegments,
												std::list<aiVector3D>& pVertices)
{
	// check argument values ranges.
    if ( ( pStartAngle < -AI_MATH_TWO_PI_F ) || ( pStartAngle > AI_MATH_TWO_PI_F ) )
    {
        Throw_ArgOutOfRange( "GeometryHelper_Make_Arc2D.pStartAngle" );
    }
    if ( ( pEndAngle < -AI_MATH_TWO_PI_F ) || ( pEndAngle > AI_MATH_TWO_PI_F ) )
    {
        Throw_ArgOutOfRange( "GeometryHelper_Make_Arc2D.pEndAngle" );
    }
    if ( pRadius <= 0 )
    {
        Throw_ArgOutOfRange( "GeometryHelper_Make_Arc2D.pRadius" );
    }

	// calculate arc angle and check type of arc
	float angle_full = std::fabs(pEndAngle - pStartAngle);
    if ( ( angle_full > AI_MATH_TWO_PI_F ) || ( angle_full == 0.0f ) )
    {
        angle_full = AI_MATH_TWO_PI_F;
    }

	// calculate angle for one step - angle to next point of line.
	float angle_step = angle_full / (float)pNumSegments;
	// make points
	for(size_t pi = 0; pi <= pNumSegments; pi++)
	{
		float tangle = pStartAngle + pi * angle_step;
		pVertices.push_back(GeometryHelper_Make_Point2D(tangle, pRadius));
	}// for(size_t pi = 0; pi <= pNumSegments; pi++)

	// if we making full circle then add last vertex equal to first vertex
	if(angle_full == AI_MATH_TWO_PI_F) pVertices.push_back(*pVertices.begin());
}

void X3DImporter::GeometryHelper_Extend_PointToLine(const std::list<aiVector3D>& pPoint, std::list<aiVector3D>& pLine)
{
    std::list<aiVector3D>::const_iterator pit = pPoint.begin();
    std::list<aiVector3D>::const_iterator pit_last = pPoint.end();

	pit_last--;

    if ( pPoint.size() < 2 )
    {
        Throw_ArgOutOfRange( "GeometryHelper_Extend_PointToLine.pPoint.size() can not be less than 2." );
    }

	// add first point of first line.
	pLine.push_back(*pit++);
	// add internal points
	while(pit != pit_last)
	{
		pLine.push_back(*pit);// second point of previous line
		pLine.push_back(*pit);// first point of next line
		pit++;
	}
	// add last point of last line
	pLine.push_back(*pit);
}

void X3DImporter::GeometryHelper_Extend_PolylineIdxToLineIdx(const std::list<int32_t>& pPolylineCoordIdx, std::list<int32_t>& pLineCoordIdx)
{
    std::list<int32_t>::const_iterator plit = pPolylineCoordIdx.begin();

	while(plit != pPolylineCoordIdx.end())
	{
		// add first point of polyline
		pLineCoordIdx.push_back(*plit++);
		while((*plit != (-1)) && (plit != pPolylineCoordIdx.end()))
		{
			std::list<int32_t>::const_iterator plit_next;

			plit_next = plit, plit_next++;
			pLineCoordIdx.push_back(*plit);// second point of previous line.
			pLineCoordIdx.push_back(-1);// delimiter
			if((*plit_next == (-1)) || (plit_next == pPolylineCoordIdx.end())) break;// current polyline is finished

			pLineCoordIdx.push_back(*plit);// first point of next line.
			plit = plit_next;
		}// while((*plit != (-1)) && (plit != pPolylineCoordIdx.end()))
	}// while(plit != pPolylineCoordIdx.end())
}

#define MESH_RectParallelepiped_CREATE_VERT \
aiVector3D vert_set[8]; \
float x1, x2, y1, y2, z1, z2, hs; \
 \
	hs = pSize.x / 2, x1 = -hs, x2 = hs; \
	hs = pSize.y / 2, y1 = -hs, y2 = hs; \
	hs = pSize.z / 2, z1 = -hs, z2 = hs; \
	vert_set[0].Set(x2, y1, z2); \
	vert_set[1].Set(x2, y2, z2); \
	vert_set[2].Set(x2, y2, z1); \
	vert_set[3].Set(x2, y1, z1); \
	vert_set[4].Set(x1, y1, z2); \
	vert_set[5].Set(x1, y2, z2); \
	vert_set[6].Set(x1, y2, z1); \
	vert_set[7].Set(x1, y1, z1)

void X3DImporter::GeometryHelper_MakeQL_RectParallelepiped(const aiVector3D& pSize, std::list<aiVector3D>& pVertices)
{
	MESH_RectParallelepiped_CREATE_VERT;
	MACRO_FACE_ADD_QUAD_FA(true, pVertices, vert_set, 3, 2, 1, 0);// front
	MACRO_FACE_ADD_QUAD_FA(true, pVertices, vert_set, 6, 7, 4, 5);// back
	MACRO_FACE_ADD_QUAD_FA(true, pVertices, vert_set, 7, 3, 0, 4);// left
	MACRO_FACE_ADD_QUAD_FA(true, pVertices, vert_set, 2, 6, 5, 1);// right
	MACRO_FACE_ADD_QUAD_FA(true, pVertices, vert_set, 0, 1, 5, 4);// top
	MACRO_FACE_ADD_QUAD_FA(true, pVertices, vert_set, 7, 6, 2, 3);// bottom
}

#undef MESH_RectParallelepiped_CREATE_VERT

void X3DImporter::GeometryHelper_CoordIdxStr2FacesArr(const std::vector<int32_t>& pCoordIdx, std::vector<aiFace>& pFaces, unsigned int& pPrimitiveTypes) const
{
    std::vector<int32_t> f_data(pCoordIdx);
    std::vector<unsigned int> inds;
    unsigned int prim_type = 0;

    if ( f_data.back() != ( -1 ) )
    {
        f_data.push_back( -1 );
    }

	// reserve average size.
	pFaces.reserve(f_data.size() / 3);
	inds.reserve(4);
    //PrintVectorSet("build. ci", pCoordIdx);
	for(std::vector<int32_t>::iterator it = f_data.begin(); it != f_data.end(); it++)
	{
		// when face is got count how many indices in it.
		if(*it == (-1))
		{
			aiFace tface;
			size_t ts;

			ts = inds.size();
			switch(ts)
			{
				case 0: goto mg_m_err;
				case 1: prim_type |= aiPrimitiveType_POINT; break;
				case 2: prim_type |= aiPrimitiveType_LINE; break;
				case 3: prim_type |= aiPrimitiveType_TRIANGLE; break;
				default: prim_type |= aiPrimitiveType_POLYGON; break;
			}

			tface.mNumIndices = static_cast<unsigned int>(ts);
			tface.mIndices = new unsigned int[ts];
			memcpy(tface.mIndices, inds.data(), ts * sizeof(unsigned int));
			pFaces.push_back(tface);
			inds.clear();
		}// if(*it == (-1))
		else
		{
			inds.push_back(*it);
		}// if(*it == (-1)) else
	}// for(std::list<int32_t>::iterator it = f_data.begin(); it != f_data.end(); it++)
//PrintVectorSet("build. faces", pCoordIdx);

	pPrimitiveTypes = prim_type;

	return;

mg_m_err:

	for(size_t i = 0, i_e = pFaces.size(); i < i_e; i++) delete [] pFaces.at(i).mIndices;

	pFaces.clear();
}

void X3DImporter::MeshGeometry_AddColor(aiMesh& pMesh, const std::list<aiColor3D>& pColors, const bool pColorPerVertex) const
{
std::list<aiColor4D> tcol;

	// create RGBA array from RGB.
	for(std::list<aiColor3D>::const_iterator it = pColors.begin(); it != pColors.end(); it++) tcol.push_back(aiColor4D((*it).r, (*it).g, (*it).b, 1));

	// call existing function for adding RGBA colors
	MeshGeometry_AddColor(pMesh, tcol, pColorPerVertex);
}

void X3DImporter::MeshGeometry_AddColor(aiMesh& pMesh, const std::list<aiColor4D>& pColors, const bool pColorPerVertex) const
{
    std::list<aiColor4D>::const_iterator col_it = pColors.begin();

	if(pColorPerVertex)
	{
		if(pColors.size() < pMesh.mNumVertices)
		{
			throw DeadlyImportError("MeshGeometry_AddColor1. Colors count(" + to_string(pColors.size()) + ") can not be less than Vertices count(" +
									to_string(pMesh.mNumVertices) +  ").");
		}

		// copy colors to mesh
		pMesh.mColors[0] = new aiColor4D[pMesh.mNumVertices];
		for(size_t i = 0; i < pMesh.mNumVertices; i++) pMesh.mColors[0][i] = *col_it++;
	}// if(pColorPerVertex)
	else
	{
		if(pColors.size() < pMesh.mNumFaces)
		{
			throw DeadlyImportError("MeshGeometry_AddColor1. Colors count(" + to_string(pColors.size()) + ") can not be less than Faces count(" +
									to_string(pMesh.mNumFaces) +  ").");
		}

		// copy colors to mesh
		pMesh.mColors[0] = new aiColor4D[pMesh.mNumVertices];
		for(size_t fi = 0; fi < pMesh.mNumFaces; fi++)
		{
			// apply color to all vertices of face
            for ( size_t vi = 0, vi_e = pMesh.mFaces[ fi ].mNumIndices; vi < vi_e; vi++ )
            {
                pMesh.mColors[ 0 ][ pMesh.mFaces[ fi ].mIndices[ vi ] ] = *col_it;
            }

			col_it++;
		}
	}// if(pColorPerVertex) else
}

void X3DImporter::MeshGeometry_AddColor(aiMesh& pMesh, const std::vector<int32_t>& pCoordIdx, const std::vector<int32_t>& pColorIdx,
										const std::list<aiColor3D>& pColors, const bool pColorPerVertex) const
{
    std::list<aiColor4D> tcol;

	// create RGBA array from RGB.
    for ( std::list<aiColor3D>::const_iterator it = pColors.begin(); it != pColors.end(); it++ )
    {
        tcol.push_back( aiColor4D( ( *it ).r, ( *it ).g, ( *it ).b, 1 ) );
    }

	// call existing function for adding RGBA colors
	MeshGeometry_AddColor(pMesh, pCoordIdx, pColorIdx, tcol, pColorPerVertex);
}

void X3DImporter::MeshGeometry_AddColor(aiMesh& pMesh, const std::vector<int32_t>& pCoordIdx, const std::vector<int32_t>& pColorIdx,
										const std::list<aiColor4D>& pColors, const bool pColorPerVertex) const
{
    std::vector<aiColor4D> col_tgt_arr;
    std::list<aiColor4D> col_tgt_list;
    std::vector<aiColor4D> col_arr_copy;

    if ( pCoordIdx.size() == 0 )
    {
        throw DeadlyImportError( "MeshGeometry_AddColor2. pCoordIdx can not be empty." );
    }

	// copy list to array because we are need indexed access to colors.
	col_arr_copy.reserve(pColors.size());
    for ( std::list<aiColor4D>::const_iterator it = pColors.begin(); it != pColors.end(); it++ )
    {
        col_arr_copy.push_back( *it );
    }

	if(pColorPerVertex)
	{
		if(pColorIdx.size() > 0)
		{
			// check indices array count.
			if(pColorIdx.size() < pCoordIdx.size())
			{
				throw DeadlyImportError("MeshGeometry_AddColor2. Colors indices count(" + to_string(pColorIdx.size()) +
										") can not be less than Coords inidces count(" + to_string(pCoordIdx.size()) +  ").");
			}
			// create list with colors for every vertex.
			col_tgt_arr.resize(pMesh.mNumVertices);
			for(std::vector<int32_t>::const_iterator colidx_it = pColorIdx.begin(), coordidx_it = pCoordIdx.begin(); colidx_it != pColorIdx.end(); colidx_it++, coordidx_it++)
			{
                if ( *colidx_it == ( -1 ) )
                {
                    continue;// skip faces delimiter
                }
                if ( ( unsigned int ) ( *coordidx_it ) > pMesh.mNumVertices )
                {
                    throw DeadlyImportError( "MeshGeometry_AddColor2. Coordinate idx is out of range." );
                }
                if ( ( unsigned int ) *colidx_it > pMesh.mNumVertices )
                {
                    throw DeadlyImportError( "MeshGeometry_AddColor2. Color idx is out of range." );
                }

				col_tgt_arr[*coordidx_it] = col_arr_copy[*colidx_it];
			}
		}// if(pColorIdx.size() > 0)
		else
		{
			// when color indices list is absent use CoordIdx.
			// check indices array count.
			if(pColors.size() < pMesh.mNumVertices)
			{
				throw DeadlyImportError("MeshGeometry_AddColor2. Colors count(" + to_string(pColors.size()) + ") can not be less than Vertices count(" +
										to_string(pMesh.mNumVertices) +  ").");
			}
			// create list with colors for every vertex.
			col_tgt_arr.resize(pMesh.mNumVertices);
            for ( size_t i = 0; i < pMesh.mNumVertices; i++ )
            {
                col_tgt_arr[ i ] = col_arr_copy[ i ];
            }
		}// if(pColorIdx.size() > 0) else
	}// if(pColorPerVertex)
	else
	{
		if(pColorIdx.size() > 0)
		{
			// check indices array count.
			if(pColorIdx.size() < pMesh.mNumFaces)
			{
				throw DeadlyImportError("MeshGeometry_AddColor2. Colors indices count(" + to_string(pColorIdx.size()) +
										") can not be less than Faces count(" + to_string(pMesh.mNumFaces) +  ").");
			}
			// create list with colors for every vertex using faces indices.
			col_tgt_arr.resize(pMesh.mNumFaces);

			std::vector<int32_t>::const_iterator colidx_it = pColorIdx.begin();
			for(size_t fi = 0; fi < pMesh.mNumFaces; fi++)
			{
				if((unsigned int)*colidx_it > pMesh.mNumFaces) throw DeadlyImportError("MeshGeometry_AddColor2. Face idx is out of range.");

				col_tgt_arr[fi] = col_arr_copy[*colidx_it++];
			}
		}// if(pColorIdx.size() > 0)
		else
		{
			// when color indices list is absent use CoordIdx.
			// check indices array count.
			if(pColors.size() < pMesh.mNumFaces)
			{
				throw DeadlyImportError("MeshGeometry_AddColor2. Colors count(" + to_string(pColors.size()) + ") can not be less than Faces count(" +
										to_string(pMesh.mNumFaces) +  ").");
			}
			// create list with colors for every vertex using faces indices.
			col_tgt_arr.resize(pMesh.mNumFaces);
			for(size_t fi = 0; fi < pMesh.mNumFaces; fi++) col_tgt_arr[fi] = col_arr_copy[fi];

		}// if(pColorIdx.size() > 0) else
	}// if(pColorPerVertex) else

	// copy array to list for calling function that add colors.
	for(std::vector<aiColor4D>::const_iterator it = col_tgt_arr.begin(); it != col_tgt_arr.end(); it++) col_tgt_list.push_back(*it);
	// add prepared colors list to mesh.
	MeshGeometry_AddColor(pMesh, col_tgt_list, pColorPerVertex);
}

void X3DImporter::MeshGeometry_AddNormal(aiMesh& pMesh, const std::vector<int32_t>& pCoordIdx, const std::vector<int32_t>& pNormalIdx,
								const std::list<aiVector3D>& pNormals, const bool pNormalPerVertex) const
{
    std::vector<size_t> tind;
    std::vector<aiVector3D> norm_arr_copy;

	// copy list to array because we are need indexed access to normals.
	norm_arr_copy.reserve(pNormals.size());
    for ( std::list<aiVector3D>::const_iterator it = pNormals.begin(); it != pNormals.end(); it++ )
    {
        norm_arr_copy.push_back( *it );
    }

	if(pNormalPerVertex)
	{
		if(pNormalIdx.size() > 0)
		{
			// check indices array count.
			if(pNormalIdx.size() != pCoordIdx.size()) throw DeadlyImportError("Normals and Coords inidces count must be equal.");

			tind.reserve(pNormalIdx.size());
			for(std::vector<int32_t>::const_iterator it = pNormalIdx.begin(); it != pNormalIdx.end(); it++)
			{
				if(*it != (-1)) tind.push_back(*it);
			}

			// copy normals to mesh
			pMesh.mNormals = new aiVector3D[pMesh.mNumVertices];
			for(size_t i = 0; (i < pMesh.mNumVertices) && (i < tind.size()); i++)
			{
				if(tind[i] >= norm_arr_copy.size())
					throw DeadlyImportError("MeshGeometry_AddNormal. Normal index(" + to_string(tind[i]) +
											") is out of range. Normals count: " + to_string(norm_arr_copy.size()) + ".");

				pMesh.mNormals[i] = norm_arr_copy[tind[i]];
			}
		}
		else
		{
			if(pNormals.size() != pMesh.mNumVertices) throw DeadlyImportError("MeshGeometry_AddNormal. Normals and vertices count must be equal.");

			// copy normals to mesh
			pMesh.mNormals = new aiVector3D[pMesh.mNumVertices];
			std::list<aiVector3D>::const_iterator norm_it = pNormals.begin();
			for(size_t i = 0; i < pMesh.mNumVertices; i++) pMesh.mNormals[i] = *norm_it++;
		}
	}// if(pNormalPerVertex)
	else
	{
		if(pNormalIdx.size() > 0)
		{
			if(pMesh.mNumFaces != pNormalIdx.size()) throw DeadlyImportError("Normals faces count must be equal to mesh faces count.");

			std::vector<int32_t>::const_iterator normidx_it = pNormalIdx.begin();

			tind.reserve(pNormalIdx.size());
			for(size_t i = 0, i_e = pNormalIdx.size(); i < i_e; i++) tind.push_back(*normidx_it++);

		}
		else
		{
			tind.reserve(pMesh.mNumFaces);
			for(size_t i = 0; i < pMesh.mNumFaces; i++) tind.push_back(i);

		}

		// copy normals to mesh
		pMesh.mNormals = new aiVector3D[pMesh.mNumVertices];
		for(size_t fi = 0; fi < pMesh.mNumFaces; fi++)
		{
			aiVector3D tnorm;

			tnorm = norm_arr_copy[tind[fi]];
			for(size_t vi = 0, vi_e = pMesh.mFaces[fi].mNumIndices; vi < vi_e; vi++) pMesh.mNormals[pMesh.mFaces[fi].mIndices[vi]] = tnorm;
		}
	}// if(pNormalPerVertex) else
}

void X3DImporter::MeshGeometry_AddNormal(aiMesh& pMesh, const std::list<aiVector3D>& pNormals, const bool pNormalPerVertex) const
{
    std::list<aiVector3D>::const_iterator norm_it = pNormals.begin();

	if(pNormalPerVertex)
	{
		if(pNormals.size() != pMesh.mNumVertices) throw DeadlyImportError("MeshGeometry_AddNormal. Normals and vertices count must be equal.");

		// copy normals to mesh
		pMesh.mNormals = new aiVector3D[pMesh.mNumVertices];
		for(size_t i = 0; i < pMesh.mNumVertices; i++) pMesh.mNormals[i] = *norm_it++;
	}// if(pNormalPerVertex)
	else
	{
		if(pNormals.size() != pMesh.mNumFaces) throw DeadlyImportError("MeshGeometry_AddNormal. Normals and faces count must be equal.");

		// copy normals to mesh
		pMesh.mNormals = new aiVector3D[pMesh.mNumVertices];
		for(size_t fi = 0; fi < pMesh.mNumFaces; fi++)
		{
			// apply color to all vertices of face
			for(size_t vi = 0, vi_e = pMesh.mFaces[fi].mNumIndices; vi < vi_e; vi++) pMesh.mNormals[pMesh.mFaces[fi].mIndices[vi]] = *norm_it;

			norm_it++;
		}
	}// if(pNormalPerVertex) else
}

void X3DImporter::MeshGeometry_AddTexCoord(aiMesh& pMesh, const std::vector<int32_t>& pCoordIdx, const std::vector<int32_t>& pTexCoordIdx,
								const std::list<aiVector2D>& pTexCoords) const
{
    std::vector<aiVector3D> texcoord_arr_copy;
    std::vector<aiFace> faces;
    unsigned int prim_type;

	// copy list to array because we are need indexed access to normals.
	texcoord_arr_copy.reserve(pTexCoords.size());
	for(std::list<aiVector2D>::const_iterator it = pTexCoords.begin(); it != pTexCoords.end(); it++)
	{
		texcoord_arr_copy.push_back(aiVector3D((*it).x, (*it).y, 0));
	}

	if(pTexCoordIdx.size() > 0)
	{
		GeometryHelper_CoordIdxStr2FacesArr(pTexCoordIdx, faces, prim_type);
        if ( faces.empty() )
        {
            throw DeadlyImportError( "Failed to add texture coordinates to mesh, faces list is empty." );
        }
        if ( faces.size() != pMesh.mNumFaces )
        {
            throw DeadlyImportError( "Texture coordinates faces count must be equal to mesh faces count." );
        }
	}
	else
	{
		GeometryHelper_CoordIdxStr2FacesArr(pCoordIdx, faces, prim_type);
	}

	pMesh.mTextureCoords[0] = new aiVector3D[pMesh.mNumVertices];
	pMesh.mNumUVComponents[0] = 2;
	for(size_t fi = 0, fi_e = faces.size(); fi < fi_e; fi++)
	{
		if(pMesh.mFaces[fi].mNumIndices != faces.at(fi).mNumIndices)
			throw DeadlyImportError("Number of indices in texture face and mesh face must be equal. Invalid face index: " + to_string(fi) + ".");

		for(size_t ii = 0; ii < pMesh.mFaces[fi].mNumIndices; ii++)
		{
			size_t vert_idx = pMesh.mFaces[fi].mIndices[ii];
			size_t tc_idx = faces.at(fi).mIndices[ii];

			pMesh.mTextureCoords[0][vert_idx] = texcoord_arr_copy.at(tc_idx);
		}
	}// for(size_t fi = 0, fi_e = faces.size(); fi < fi_e; fi++)
}

void X3DImporter::MeshGeometry_AddTexCoord(aiMesh& pMesh, const std::list<aiVector2D>& pTexCoords) const
{
    std::vector<aiVector3D> tc_arr_copy;

    if ( pTexCoords.size() != pMesh.mNumVertices )
    {
        throw DeadlyImportError( "MeshGeometry_AddTexCoord. Texture coordinates and vertices count must be equal." );
    }

	// copy list to array because we are need convert aiVector2D to aiVector3D and also get indexed access as a bonus.
	tc_arr_copy.reserve(pTexCoords.size());
    for ( std::list<aiVector2D>::const_iterator it = pTexCoords.begin(); it != pTexCoords.end(); it++ )
    {
        tc_arr_copy.push_back( aiVector3D( ( *it ).x, ( *it ).y, 0 ) );
    }

	// copy texture coordinates to mesh
	pMesh.mTextureCoords[0] = new aiVector3D[pMesh.mNumVertices];
	pMesh.mNumUVComponents[0] = 2;
    for ( size_t i = 0; i < pMesh.mNumVertices; i++ )
    {
        pMesh.mTextureCoords[ 0 ][ i ] = tc_arr_copy[ i ];
    }
}

aiMesh* X3DImporter::GeometryHelper_MakeMesh(const std::vector<int32_t>& pCoordIdx, const std::list<aiVector3D>& pVertices) const
{
    std::vector<aiFace> faces;
    unsigned int prim_type = 0;

	// create faces array from input string with vertices indices.
	GeometryHelper_CoordIdxStr2FacesArr(pCoordIdx, faces, prim_type);
    if ( !faces.size() )
    {
        throw DeadlyImportError( "Failed to create mesh, faces list is empty." );
    }

	//
	// Create new mesh and copy geometry data.
	//
    aiMesh *tmesh = new aiMesh;
    size_t ts = faces.size();
	// faces
	tmesh->mFaces = new aiFace[ts];
	tmesh->mNumFaces = static_cast<unsigned int>(ts);
	for(size_t i = 0; i < ts; i++) tmesh->mFaces[i] = faces.at(i);

	// vertices
	std::list<aiVector3D>::const_iterator vit = pVertices.begin();

	ts = pVertices.size();
	tmesh->mVertices = new aiVector3D[ts];
	tmesh->mNumVertices = static_cast<unsigned int>(ts);
    for ( size_t i = 0; i < ts; i++ )
    {
        tmesh->mVertices[ i ] = *vit++;
    }

	// set primitives type and return result.
	tmesh->mPrimitiveTypes = prim_type;

	return tmesh;
}

/*********************************************************************************************************************************************/
/************************************************************ Functions: parse set ***********************************************************/
/*********************************************************************************************************************************************/

void X3DImporter::ParseHelper_Group_Begin(const bool pStatic)
{
    CX3DImporter_NodeElement_Group* new_group = new CX3DImporter_NodeElement_Group(NodeElement_Cur, pStatic);// create new node with current node as parent.

	// if we are adding not the root element then add new element to current element child list.
    if ( NodeElement_Cur != nullptr )
    {
        NodeElement_Cur->Child.push_back( new_group );
    }

	NodeElement_List.push_back(new_group);// it's a new element - add it to list.
	NodeElement_Cur = new_group;// switch current element to new one.
}

void X3DImporter::ParseHelper_Node_Enter(CX3DImporter_NodeElement* pNode)
{
	NodeElement_Cur->Child.push_back(pNode);// add new element to current element child list.
	NodeElement_Cur = pNode;// switch current element to new one.
}

void X3DImporter::ParseHelper_Node_Exit()
{
	// check if we can walk up.
    if ( NodeElement_Cur != nullptr )
    {
        NodeElement_Cur = NodeElement_Cur->Parent;
    }
}

void X3DImporter::ParseHelper_FixTruncatedFloatString(const char* pInStr, std::string& pOutString)
{
	pOutString.clear();
    const size_t instr_len = strlen(pInStr);
    if ( 0 == instr_len )
    {
        return;
    }

	pOutString.reserve(instr_len * 3 / 2);
	// check and correct floats in format ".x". Must be "x.y".
    if ( pInStr[ 0 ] == '.' )
    {
        pOutString.push_back( '0' );
    }

	pOutString.push_back(pInStr[0]);
	for(size_t ci = 1; ci < instr_len; ci++)
	{
		if((pInStr[ci] == '.') && ((pInStr[ci - 1] == ' ') || (pInStr[ci - 1] == '-') || (pInStr[ci - 1] == '+') || (pInStr[ci - 1] == '\t')))
		{
			pOutString.push_back('0');
			pOutString.push_back('.');
		}
		else
		{
			pOutString.push_back(pInStr[ci]);
		}
	}
}

extern FIVocabulary X3D_vocabulary_3_2;
extern FIVocabulary X3D_vocabulary_3_3;

void X3DImporter::ParseFile(const std::string& pFile, IOSystem* pIOHandler)
{
    std::unique_ptr<FIReader> OldReader = std::move(mReader);// store current XMLreader.
    std::unique_ptr<IOStream> file(pIOHandler->Open(pFile, "rb"));

	// Check whether we can read from the file
    if ( file.get() == nullptr )
    {
        throw DeadlyImportError( "Failed to open X3D file " + pFile + "." );
    }
	mReader = FIReader::create(file.get());
    if ( !mReader )
    {
        throw DeadlyImportError( "Failed to create XML reader for file" + pFile + "." );
    }
    mReader->registerVocabulary("urn:web3d:x3d:fi-vocabulary-3.2", &X3D_vocabulary_3_2);
    mReader->registerVocabulary("urn:web3d:x3d:fi-vocabulary-3.3", &X3D_vocabulary_3_3);
	// start reading
	ParseNode_Root();

	// restore old XMLreader
	mReader = std::move(OldReader);
}

void X3DImporter::ParseNode_Root()
{
	// search for root tag <X3D>
    if ( !XML_SearchNode( "X3D" ) )
    {
        throw DeadlyImportError( "Root node \"X3D\" not found." );
    }

	ParseHelper_Group_Begin();// create root node element.
	// parse other contents
	while(mReader->read())
	{
        if ( mReader->getNodeType() != irr::io::EXN_ELEMENT )
        {
            continue;
        }

		if(XML_CheckNode_NameEqual("head"))
			ParseNode_Head();
		else if(XML_CheckNode_NameEqual("Scene"))
			ParseNode_Scene();
		else
			XML_CheckNode_SkipUnsupported("Root");
	}

	// exit from root node element.
	ParseHelper_Node_Exit();
}

void X3DImporter::ParseNode_Head()
{
    bool close_found = false;// flag: true if close tag of node are found.

	while(mReader->read())
	{
		if(mReader->getNodeType() == irr::io::EXN_ELEMENT)
		{
			if(XML_CheckNode_NameEqual("meta"))
			{
				XML_CheckNode_MustBeEmpty();

				// adding metadata from <head> as MetaString from <Scene>
                bool added( false );
                CX3DImporter_NodeElement_MetaString* ms = new CX3DImporter_NodeElement_MetaString(NodeElement_Cur);

				ms->Name = mReader->getAttributeValueSafe("name");
				// name must not be empty
				if(!ms->Name.empty())
				{
					ms->Value.push_back(mReader->getAttributeValueSafe("content"));
					NodeElement_List.push_back(ms);
                    if ( NodeElement_Cur != nullptr )
                    {
                        NodeElement_Cur->Child.push_back( ms );
                        added = true;
                    }
				}
                // if an error has occurred, release instance
                if ( !added ) {
                    delete ms;
                }
			}// if(XML_CheckNode_NameEqual("meta"))
		}// if(mReader->getNodeType() == irr::io::EXN_ELEMENT)
		else if(mReader->getNodeType() == irr::io::EXN_ELEMENT_END)
		{
			if(XML_CheckNode_NameEqual("head"))
			{
				close_found = true;
				break;
			}
		}// if(mReader->getNodeType() == irr::io::EXN_ELEMENT) else
	}// while(mReader->read())

    if ( !close_found )
    {
        Throw_CloseNotFound( "head" );
    }
}

void X3DImporter::ParseNode_Scene()
{
    auto GroupCounter_Increase = [](size_t& pCounter, const char* pGroupName) -> void
    {
	    pCounter++;
	    if(pCounter == 0) throw DeadlyImportError("Group counter overflow. Too much groups with type: " + std::string(pGroupName) + ".");
};

auto GroupCounter_Decrease = [&](size_t& pCounter, const char* pGroupName) -> void
{
	if(pCounter == 0) Throw_TagCountIncorrect(pGroupName);

	pCounter--;
};

static const char* GroupName_Group = "Group";
static const char* GroupName_StaticGroup = "StaticGroup";
static const char* GroupName_Transform = "Transform";
static const char* GroupName_Switch = "Switch";

bool close_found = false;
size_t counter_group = 0;
size_t counter_transform = 0;
size_t counter_switch = 0;

	// while create static node? Because objects name used deeper in "USE" attribute can be equal to some meta in <head> node.
	ParseHelper_Group_Begin(true);
	while(mReader->read())
	{
		if(mReader->getNodeType() == irr::io::EXN_ELEMENT)
		{
			if(XML_CheckNode_NameEqual("Shape"))
			{
				ParseNode_Shape_Shape();
			}
			else if(XML_CheckNode_NameEqual(GroupName_Group))
			{
				GroupCounter_Increase(counter_group, GroupName_Group);
				ParseNode_Grouping_Group();
				// if node is empty then decrease group counter at this place.
				if(mReader->isEmptyElement()) GroupCounter_Decrease(counter_group, GroupName_Group);
			}
			else if(XML_CheckNode_NameEqual(GroupName_StaticGroup))
			{
				GroupCounter_Increase(counter_group, GroupName_StaticGroup);
				ParseNode_Grouping_StaticGroup();
				// if node is empty then decrease group counter at this place.
				if(mReader->isEmptyElement()) GroupCounter_Decrease(counter_group, GroupName_StaticGroup);
			}
			else if(XML_CheckNode_NameEqual(GroupName_Transform))
			{
				GroupCounter_Increase(counter_transform, GroupName_Transform);
				ParseNode_Grouping_Transform();
				// if node is empty then decrease group counter at this place.
				if(mReader->isEmptyElement()) GroupCounter_Decrease(counter_transform, GroupName_Transform);
			}
			else if(XML_CheckNode_NameEqual(GroupName_Switch))
			{
				GroupCounter_Increase(counter_switch, GroupName_Switch);
				ParseNode_Grouping_Switch();
				// if node is empty then decrease group counter at this place.
				if(mReader->isEmptyElement()) GroupCounter_Decrease(counter_switch, GroupName_Switch);
			}
			else if(XML_CheckNode_NameEqual("DirectionalLight"))
		{
				ParseNode_Lighting_DirectionalLight();
			}
			else if(XML_CheckNode_NameEqual("PointLight"))
			{
				ParseNode_Lighting_PointLight();
			}
			else if(XML_CheckNode_NameEqual("SpotLight"))
			{
				ParseNode_Lighting_SpotLight();
			}
			else if(XML_CheckNode_NameEqual("Inline"))
			{
				ParseNode_Networking_Inline();
			}
			else if(!ParseHelper_CheckRead_X3DMetadataObject())
			{
				XML_CheckNode_SkipUnsupported("Scene");
			}
		}// if(mReader->getNodeType() == irr::io::EXN_ELEMENT)
		else if(mReader->getNodeType() == irr::io::EXN_ELEMENT_END)
		{
			if(XML_CheckNode_NameEqual("Scene"))
			{
				close_found = true;

				break;
			}
			else if(XML_CheckNode_NameEqual(GroupName_Group))
			{
				GroupCounter_Decrease(counter_group, GroupName_Group);
				ParseNode_Grouping_GroupEnd();
			}
			else if(XML_CheckNode_NameEqual(GroupName_StaticGroup))
			{
				GroupCounter_Decrease(counter_group, GroupName_StaticGroup);
				ParseNode_Grouping_StaticGroupEnd();
			}
			else if(XML_CheckNode_NameEqual(GroupName_Transform))
			{
				GroupCounter_Decrease(counter_transform, GroupName_Transform);
				ParseNode_Grouping_TransformEnd();
			}
			else if(XML_CheckNode_NameEqual(GroupName_Switch))
			{
				GroupCounter_Decrease(counter_switch, GroupName_Switch);
				ParseNode_Grouping_SwitchEnd();
			}
		}// if(mReader->getNodeType() == irr::io::EXN_ELEMENT) else
	}// while(mReader->read())

	ParseHelper_Node_Exit();

	if(counter_group) Throw_TagCountIncorrect("Group");
	if(counter_transform) Throw_TagCountIncorrect("Transform");
	if(counter_switch) Throw_TagCountIncorrect("Switch");
	if(!close_found) Throw_CloseNotFound("Scene");

}

/*********************************************************************************************************************************************/
/******************************************************** Functions: BaseImporter set ********************************************************/
/*********************************************************************************************************************************************/

bool X3DImporter::CanRead(const std::string& pFile, IOSystem* pIOHandler, bool pCheckSig) const
{
    const std::string extension = GetExtension(pFile);

	if((extension == "x3d") || (extension == "x3db")) return true;

	if(!extension.length() || pCheckSig)
	{
		const char* tokens[] = { "DOCTYPE X3D PUBLIC", "http://www.web3d.org/specifications/x3d" };

		return SearchFileHeaderForToken(pIOHandler, pFile, tokens, 2);
	}

	return false;
}

void X3DImporter::GetExtensionList(std::set<std::string>& pExtensionList)
{
	pExtensionList.insert("x3d");
	pExtensionList.insert("x3db");
}

const aiImporterDesc* X3DImporter::GetInfo () const
{
	return &Description;
}

void X3DImporter::InternReadFile(const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler)
{
	mpIOHandler = pIOHandler;

	Clear();// delete old graph.
	std::string::size_type slashPos = pFile.find_last_of("\\/");
	pIOHandler->PushDirectory(slashPos == std::string::npos ? std::string() : pFile.substr(0, slashPos + 1));
	ParseFile(pFile, pIOHandler);
	pIOHandler->PopDirectory();
	//
	// Assimp use static arrays of objects for fast speed of rendering. That's good, but need some additional operations/
	// We know that geometry objects(meshes) are stored in <Shape>, also in <Shape>-><Appearance> materials(in Assimp logical view)
	// are stored. So at first we need to count how meshes and materials are stored in scene graph.
	//
	// at first creating root node for aiScene.
	pScene->mRootNode = new aiNode;
	pScene->mRootNode->mParent = nullptr;
	pScene->mFlags |= AI_SCENE_FLAGS_ALLOW_SHARED;
	//search for root node element
	NodeElement_Cur = NodeElement_List.front();
	while(NodeElement_Cur->Parent != nullptr) NodeElement_Cur = NodeElement_Cur->Parent;

	{// fill aiScene with objects.
		std::list<aiMesh*> mesh_list;
		std::list<aiMaterial*> mat_list;
		std::list<aiLight*> light_list;

		// create nodes tree
		Postprocess_BuildNode(*NodeElement_Cur, *pScene->mRootNode, mesh_list, mat_list, light_list);
		// copy needed data to scene
		if(mesh_list.size() > 0)
		{
			std::list<aiMesh*>::const_iterator it = mesh_list.begin();

			pScene->mNumMeshes = static_cast<unsigned int>(mesh_list.size());
			pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];
			for(size_t i = 0; i < pScene->mNumMeshes; i++) pScene->mMeshes[i] = *it++;
		}

		if(mat_list.size() > 0)
		{
			std::list<aiMaterial*>::const_iterator it = mat_list.begin();

			pScene->mNumMaterials = static_cast<unsigned int>(mat_list.size());
			pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
			for(size_t i = 0; i < pScene->mNumMaterials; i++) pScene->mMaterials[i] = *it++;
		}

		if(light_list.size() > 0)
		{
			std::list<aiLight*>::const_iterator it = light_list.begin();

			pScene->mNumLights = static_cast<unsigned int>(light_list.size());
			pScene->mLights = new aiLight*[pScene->mNumLights];
			for(size_t i = 0; i < pScene->mNumLights; i++) pScene->mLights[i] = *it++;
		}
	}// END: fill aiScene with objects.

	///TODO: IME optimize tree
}

}// namespace Assimp

#endif // !ASSIMP_BUILD_NO_X3D_IMPORTER
