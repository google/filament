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
/// \file   X3DImporter_Postprocess.cpp
/// \brief  Convert built scenegraph and objects to Assimp scenegraph.
/// \date   2015-2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_X3D_IMPORTER

#include "X3DImporter.hpp"

// Header files, Assimp.
#include <assimp/ai_assert.h>
#include <assimp/StandardShapes.h>
#include <assimp/StringUtils.h>

// Header files, stdlib.
#include <algorithm>
#include <iterator>
#include <string>

namespace Assimp
{

aiMatrix4x4 X3DImporter::PostprocessHelper_Matrix_GlobalToCurrent() const
{
    CX3DImporter_NodeElement* cur_node;
    std::list<aiMatrix4x4> matr;
    aiMatrix4x4 out_matr;

	// starting walk from current element to root
	cur_node = NodeElement_Cur;
	if(cur_node != nullptr)
	{
		do
		{
			// if cur_node is group then store group transformation matrix in list.
			if(cur_node->Type == CX3DImporter_NodeElement::ENET_Group) matr.push_back(((CX3DImporter_NodeElement_Group*)cur_node)->Transformation);

			cur_node = cur_node->Parent;
		} while(cur_node != nullptr);
	}

	// multiplicate all matrices in reverse order
	for(std::list<aiMatrix4x4>::reverse_iterator rit = matr.rbegin(); rit != matr.rend(); rit++) out_matr = out_matr * (*rit);

	return out_matr;
}

void X3DImporter::PostprocessHelper_CollectMetadata(const CX3DImporter_NodeElement& pNodeElement, std::list<CX3DImporter_NodeElement*>& pList) const
{
	// walk through childs and find for metadata.
	for(std::list<CX3DImporter_NodeElement*>::const_iterator el_it = pNodeElement.Child.begin(); el_it != pNodeElement.Child.end(); el_it++)
	{
		if(((*el_it)->Type == CX3DImporter_NodeElement::ENET_MetaBoolean) || ((*el_it)->Type == CX3DImporter_NodeElement::ENET_MetaDouble) ||
			((*el_it)->Type == CX3DImporter_NodeElement::ENET_MetaFloat) || ((*el_it)->Type == CX3DImporter_NodeElement::ENET_MetaInteger) ||
			((*el_it)->Type == CX3DImporter_NodeElement::ENET_MetaString))
		{
			pList.push_back(*el_it);
		}
		else if((*el_it)->Type == CX3DImporter_NodeElement::ENET_MetaSet)
		{
			PostprocessHelper_CollectMetadata(**el_it, pList);
		}
	}// for(std::list<CX3DImporter_NodeElement*>::const_iterator el_it = pNodeElement.Child.begin(); el_it != pNodeElement.Child.end(); el_it++)
}

bool X3DImporter::PostprocessHelper_ElementIsMetadata(const CX3DImporter_NodeElement::EType pType) const
{
	if((pType == CX3DImporter_NodeElement::ENET_MetaBoolean) || (pType == CX3DImporter_NodeElement::ENET_MetaDouble) ||
		(pType == CX3DImporter_NodeElement::ENET_MetaFloat) || (pType == CX3DImporter_NodeElement::ENET_MetaInteger) ||
		(pType == CX3DImporter_NodeElement::ENET_MetaString) || (pType == CX3DImporter_NodeElement::ENET_MetaSet))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool X3DImporter::PostprocessHelper_ElementIsMesh(const CX3DImporter_NodeElement::EType pType) const
{
	if((pType == CX3DImporter_NodeElement::ENET_Arc2D) || (pType == CX3DImporter_NodeElement::ENET_ArcClose2D) ||
		(pType == CX3DImporter_NodeElement::ENET_Box) || (pType == CX3DImporter_NodeElement::ENET_Circle2D) ||
		(pType == CX3DImporter_NodeElement::ENET_Cone) || (pType == CX3DImporter_NodeElement::ENET_Cylinder) ||
		(pType == CX3DImporter_NodeElement::ENET_Disk2D) || (pType == CX3DImporter_NodeElement::ENET_ElevationGrid) ||
		(pType == CX3DImporter_NodeElement::ENET_Extrusion) || (pType == CX3DImporter_NodeElement::ENET_IndexedFaceSet) ||
		(pType == CX3DImporter_NodeElement::ENET_IndexedLineSet) || (pType == CX3DImporter_NodeElement::ENET_IndexedTriangleFanSet) ||
		(pType == CX3DImporter_NodeElement::ENET_IndexedTriangleSet) || (pType == CX3DImporter_NodeElement::ENET_IndexedTriangleStripSet) ||
		(pType == CX3DImporter_NodeElement::ENET_PointSet) || (pType == CX3DImporter_NodeElement::ENET_LineSet) ||
		(pType == CX3DImporter_NodeElement::ENET_Polyline2D) || (pType == CX3DImporter_NodeElement::ENET_Polypoint2D) ||
		(pType == CX3DImporter_NodeElement::ENET_Rectangle2D) || (pType == CX3DImporter_NodeElement::ENET_Sphere) ||
		(pType == CX3DImporter_NodeElement::ENET_TriangleFanSet) || (pType == CX3DImporter_NodeElement::ENET_TriangleSet) ||
		(pType == CX3DImporter_NodeElement::ENET_TriangleSet2D) || (pType == CX3DImporter_NodeElement::ENET_TriangleStripSet))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void X3DImporter::Postprocess_BuildLight(const CX3DImporter_NodeElement& pNodeElement, std::list<aiLight*>& pSceneLightList) const
{
    const CX3DImporter_NodeElement_Light& ne = *( ( CX3DImporter_NodeElement_Light* ) &pNodeElement );
    aiMatrix4x4 transform_matr = PostprocessHelper_Matrix_GlobalToCurrent();
    aiLight* new_light = new aiLight;

	new_light->mName = ne.ID;
	new_light->mColorAmbient = ne.Color * ne.AmbientIntensity;
	new_light->mColorDiffuse = ne.Color * ne.Intensity;
	new_light->mColorSpecular = ne.Color * ne.Intensity;
	switch(pNodeElement.Type)
	{
		case CX3DImporter_NodeElement::ENET_DirectionalLight:
			new_light->mType = aiLightSource_DIRECTIONAL;
			new_light->mDirection = ne.Direction, new_light->mDirection *= transform_matr;

			break;
		case CX3DImporter_NodeElement::ENET_PointLight:
			new_light->mType = aiLightSource_POINT;
			new_light->mPosition = ne.Location, new_light->mPosition *= transform_matr;
			new_light->mAttenuationConstant = ne.Attenuation.x;
			new_light->mAttenuationLinear = ne.Attenuation.y;
			new_light->mAttenuationQuadratic = ne.Attenuation.z;

			break;
		case CX3DImporter_NodeElement::ENET_SpotLight:
			new_light->mType = aiLightSource_SPOT;
			new_light->mPosition = ne.Location, new_light->mPosition *= transform_matr;
			new_light->mDirection = ne.Direction, new_light->mDirection *= transform_matr;
			new_light->mAttenuationConstant = ne.Attenuation.x;
			new_light->mAttenuationLinear = ne.Attenuation.y;
			new_light->mAttenuationQuadratic = ne.Attenuation.z;
			new_light->mAngleInnerCone = ne.BeamWidth;
			new_light->mAngleOuterCone = ne.CutOffAngle;

			break;
		default:
			throw DeadlyImportError("Postprocess_BuildLight. Unknown type of light: " + to_string(pNodeElement.Type) + ".");
	}

	pSceneLightList.push_back(new_light);
}

void X3DImporter::Postprocess_BuildMaterial(const CX3DImporter_NodeElement& pNodeElement, aiMaterial** pMaterial) const
{
	// check argument
	if(pMaterial == nullptr) throw DeadlyImportError("Postprocess_BuildMaterial. pMaterial is nullptr.");
	if(*pMaterial != nullptr) throw DeadlyImportError("Postprocess_BuildMaterial. *pMaterial must be nullptr.");

	*pMaterial = new aiMaterial;
	aiMaterial& taimat = **pMaterial;// creating alias for convenience.

	// at this point pNodeElement point to <Appearance> node. Walk through childs and add all stored data.
	for(std::list<CX3DImporter_NodeElement*>::const_iterator el_it = pNodeElement.Child.begin(); el_it != pNodeElement.Child.end(); el_it++)
	{
		if((*el_it)->Type == CX3DImporter_NodeElement::ENET_Material)
		{
			aiColor3D tcol3;
			float tvalf;
			CX3DImporter_NodeElement_Material& tnemat = *((CX3DImporter_NodeElement_Material*)*el_it);

			tcol3.r = tnemat.AmbientIntensity, tcol3.g = tnemat.AmbientIntensity, tcol3.b = tnemat.AmbientIntensity;
			taimat.AddProperty(&tcol3, 1, AI_MATKEY_COLOR_AMBIENT);
			taimat.AddProperty(&tnemat.DiffuseColor, 1, AI_MATKEY_COLOR_DIFFUSE);
			taimat.AddProperty(&tnemat.EmissiveColor, 1, AI_MATKEY_COLOR_EMISSIVE);
			taimat.AddProperty(&tnemat.SpecularColor, 1, AI_MATKEY_COLOR_SPECULAR);
			tvalf = 1;
			taimat.AddProperty(&tvalf, 1, AI_MATKEY_SHININESS_STRENGTH);
			taimat.AddProperty(&tnemat.Shininess, 1, AI_MATKEY_SHININESS);
			tvalf = 1.0f - tnemat.Transparency;
			taimat.AddProperty(&tvalf, 1, AI_MATKEY_OPACITY);
		}// if((*el_it)->Type == CX3DImporter_NodeElement::ENET_Material)
		else if((*el_it)->Type == CX3DImporter_NodeElement::ENET_ImageTexture)
		{
			CX3DImporter_NodeElement_ImageTexture& tnetex = *((CX3DImporter_NodeElement_ImageTexture*)*el_it);
			aiString url_str(tnetex.URL.c_str());
			int mode = aiTextureOp_Multiply;

			taimat.AddProperty(&url_str, AI_MATKEY_TEXTURE_DIFFUSE(0));
			taimat.AddProperty(&tnetex.RepeatS, 1, AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0));
			taimat.AddProperty(&tnetex.RepeatT, 1, AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0));
			taimat.AddProperty(&mode, 1, AI_MATKEY_TEXOP_DIFFUSE(0));
		}// else if((*el_it)->Type == CX3DImporter_NodeElement::ENET_ImageTexture)
		else if((*el_it)->Type == CX3DImporter_NodeElement::ENET_TextureTransform)
		{
			aiUVTransform trans;
			CX3DImporter_NodeElement_TextureTransform& tnetextr = *((CX3DImporter_NodeElement_TextureTransform*)*el_it);

			trans.mTranslation = tnetextr.Translation - tnetextr.Center;
			trans.mScaling = tnetextr.Scale;
			trans.mRotation = tnetextr.Rotation;
			taimat.AddProperty(&trans, 1, AI_MATKEY_UVTRANSFORM_DIFFUSE(0));
		}// else if((*el_it)->Type == CX3DImporter_NodeElement::ENET_TextureTransform)
	}// for(std::list<CX3DImporter_NodeElement*>::const_iterator el_it = pNodeElement.Child.begin(); el_it != pNodeElement.Child.end(); el_it++)
}

void X3DImporter::Postprocess_BuildMesh(const CX3DImporter_NodeElement& pNodeElement, aiMesh** pMesh) const
{
	// check argument
	if(pMesh == nullptr) throw DeadlyImportError("Postprocess_BuildMesh. pMesh is nullptr.");
	if(*pMesh != nullptr) throw DeadlyImportError("Postprocess_BuildMesh. *pMesh must be nullptr.");

	/************************************************************************************************************************************/
	/************************************************************ Geometry2D ************************************************************/
	/************************************************************************************************************************************/
	if((pNodeElement.Type == CX3DImporter_NodeElement::ENET_Arc2D) || (pNodeElement.Type == CX3DImporter_NodeElement::ENET_ArcClose2D) ||
		(pNodeElement.Type == CX3DImporter_NodeElement::ENET_Circle2D) || (pNodeElement.Type == CX3DImporter_NodeElement::ENET_Disk2D) ||
		(pNodeElement.Type == CX3DImporter_NodeElement::ENET_Polyline2D) || (pNodeElement.Type == CX3DImporter_NodeElement::ENET_Polypoint2D) ||
		(pNodeElement.Type == CX3DImporter_NodeElement::ENET_Rectangle2D) || (pNodeElement.Type == CX3DImporter_NodeElement::ENET_TriangleSet2D))
	{
		CX3DImporter_NodeElement_Geometry2D& tnemesh = *((CX3DImporter_NodeElement_Geometry2D*)&pNodeElement);// create alias for convenience
		std::vector<aiVector3D> tarr;

		tarr.reserve(tnemesh.Vertices.size());
		for(std::list<aiVector3D>::iterator it = tnemesh.Vertices.begin(); it != tnemesh.Vertices.end(); it++) tarr.push_back(*it);
		*pMesh = StandardShapes::MakeMesh(tarr, static_cast<unsigned int>(tnemesh.NumIndices));// create mesh from vertices using Assimp help.

		return;// mesh is build, nothing to do anymore.
	}
	/************************************************************************************************************************************/
	/************************************************************ Geometry3D ************************************************************/
	/************************************************************************************************************************************/
	//
	// Predefined figures
	//
	if((pNodeElement.Type == CX3DImporter_NodeElement::ENET_Box) || (pNodeElement.Type == CX3DImporter_NodeElement::ENET_Cone) ||
		(pNodeElement.Type == CX3DImporter_NodeElement::ENET_Cylinder) || (pNodeElement.Type == CX3DImporter_NodeElement::ENET_Sphere))
	{
		CX3DImporter_NodeElement_Geometry3D& tnemesh = *((CX3DImporter_NodeElement_Geometry3D*)&pNodeElement);// create alias for convenience
		std::vector<aiVector3D> tarr;

		tarr.reserve(tnemesh.Vertices.size());
		for(std::list<aiVector3D>::iterator it = tnemesh.Vertices.begin(); it != tnemesh.Vertices.end(); it++) tarr.push_back(*it);

		*pMesh = StandardShapes::MakeMesh(tarr, static_cast<unsigned int>(tnemesh.NumIndices));// create mesh from vertices using Assimp help.

		return;// mesh is build, nothing to do anymore.
	}
	//
	// Parametric figures
	//
	if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_ElevationGrid)
	{
		CX3DImporter_NodeElement_ElevationGrid& tnemesh = *((CX3DImporter_NodeElement_ElevationGrid*)&pNodeElement);// create alias for convenience

		// at first create mesh from existing vertices.
		*pMesh = GeometryHelper_MakeMesh(tnemesh.CoordIdx, tnemesh.Vertices);
		// copy additional information from children
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Color)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_Color*)*ch_it)->Value, tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_ColorRGBA)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_ColorRGBA*)*ch_it)->Value, tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Normal)
				MeshGeometry_AddNormal(**pMesh,  ((CX3DImporter_NodeElement_Normal*)*ch_it)->Value, tnemesh.NormalPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_TextureCoordinate)
				MeshGeometry_AddTexCoord(**pMesh, ((CX3DImporter_NodeElement_TextureCoordinate*)*ch_it)->Value);
			else
				throw DeadlyImportError("Postprocess_BuildMesh. Unknown child of ElevationGrid: " + to_string((*ch_it)->Type) + ".");
		}// for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)

		return;// mesh is build, nothing to do anymore.
	}// if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_ElevationGrid)
	//
	// Indexed primitives sets
	//
	if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_IndexedFaceSet)
	{
		CX3DImporter_NodeElement_IndexedSet& tnemesh = *((CX3DImporter_NodeElement_IndexedSet*)&pNodeElement);// create alias for convenience

		// at first search for <Coordinate> node and create mesh.
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
			{
				*pMesh = GeometryHelper_MakeMesh(tnemesh.CoordIndex, ((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value);
			}
		}

		// copy additional information from children
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Color)
				MeshGeometry_AddColor(**pMesh, tnemesh.CoordIndex, tnemesh.ColorIndex, ((CX3DImporter_NodeElement_Color*)*ch_it)->Value, tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_ColorRGBA)
				MeshGeometry_AddColor(**pMesh, tnemesh.CoordIndex, tnemesh.ColorIndex, ((CX3DImporter_NodeElement_ColorRGBA*)*ch_it)->Value,
										tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
				{} // skip because already read when mesh created.
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Normal)
				MeshGeometry_AddNormal(**pMesh, tnemesh.CoordIndex, tnemesh.NormalIndex, ((CX3DImporter_NodeElement_Normal*)*ch_it)->Value,
										tnemesh.NormalPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_TextureCoordinate)
				MeshGeometry_AddTexCoord(**pMesh, tnemesh.CoordIndex, tnemesh.TexCoordIndex, ((CX3DImporter_NodeElement_TextureCoordinate*)*ch_it)->Value);
			else
				throw DeadlyImportError("Postprocess_BuildMesh. Unknown child of IndexedFaceSet: " + to_string((*ch_it)->Type) + ".");
		}// for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)

		return;// mesh is build, nothing to do anymore.
	}// if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_IndexedFaceSet)

	if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_IndexedLineSet)
	{
		CX3DImporter_NodeElement_IndexedSet& tnemesh = *((CX3DImporter_NodeElement_IndexedSet*)&pNodeElement);// create alias for convenience

		// at first search for <Coordinate> node and create mesh.
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
			{
				*pMesh = GeometryHelper_MakeMesh(tnemesh.CoordIndex, ((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value);
			}
		}

		// copy additional information from children
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			ai_assert(*pMesh);
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Color)
				MeshGeometry_AddColor(**pMesh, tnemesh.CoordIndex, tnemesh.ColorIndex, ((CX3DImporter_NodeElement_Color*)*ch_it)->Value, tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_ColorRGBA)
				MeshGeometry_AddColor(**pMesh, tnemesh.CoordIndex, tnemesh.ColorIndex, ((CX3DImporter_NodeElement_ColorRGBA*)*ch_it)->Value,
										tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
				{} // skip because already read when mesh created.
			else
				throw DeadlyImportError("Postprocess_BuildMesh. Unknown child of IndexedLineSet: " + to_string((*ch_it)->Type) + ".");
		}// for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)

		return;// mesh is build, nothing to do anymore.
	}// if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_IndexedLineSet)

	if((pNodeElement.Type == CX3DImporter_NodeElement::ENET_IndexedTriangleSet) ||
		(pNodeElement.Type == CX3DImporter_NodeElement::ENET_IndexedTriangleFanSet) ||
		(pNodeElement.Type == CX3DImporter_NodeElement::ENET_IndexedTriangleStripSet))
	{
		CX3DImporter_NodeElement_IndexedSet& tnemesh = *((CX3DImporter_NodeElement_IndexedSet*)&pNodeElement);// create alias for convenience

		// at first search for <Coordinate> node and create mesh.
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
			{
				*pMesh = GeometryHelper_MakeMesh(tnemesh.CoordIndex, ((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value);
			}
		}

		// copy additional information from children
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			ai_assert(*pMesh);
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Color)
				MeshGeometry_AddColor(**pMesh, tnemesh.CoordIndex, tnemesh.ColorIndex, ((CX3DImporter_NodeElement_Color*)*ch_it)->Value, tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_ColorRGBA)
				MeshGeometry_AddColor(**pMesh, tnemesh.CoordIndex, tnemesh.ColorIndex, ((CX3DImporter_NodeElement_ColorRGBA*)*ch_it)->Value,
										tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
				{} // skip because already read when mesh created.
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Normal)
				MeshGeometry_AddNormal(**pMesh, tnemesh.CoordIndex, tnemesh.NormalIndex, ((CX3DImporter_NodeElement_Normal*)*ch_it)->Value,
										tnemesh.NormalPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_TextureCoordinate)
				MeshGeometry_AddTexCoord(**pMesh, tnemesh.CoordIndex, tnemesh.TexCoordIndex, ((CX3DImporter_NodeElement_TextureCoordinate*)*ch_it)->Value);
			else
				throw DeadlyImportError("Postprocess_BuildMesh. Unknown child of IndexedTriangleSet or IndexedTriangleFanSet, or \
																	IndexedTriangleStripSet: " + to_string((*ch_it)->Type) + ".");
		}// for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)

		return;// mesh is build, nothing to do anymore.
	}// if((pNodeElement.Type == CX3DImporter_NodeElement::ENET_IndexedTriangleFanSet) || (pNodeElement.Type == CX3DImporter_NodeElement::ENET_IndexedTriangleStripSet))

	if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_Extrusion)
	{
		CX3DImporter_NodeElement_IndexedSet& tnemesh = *((CX3DImporter_NodeElement_IndexedSet*)&pNodeElement);// create alias for convenience

		*pMesh = GeometryHelper_MakeMesh(tnemesh.CoordIndex, tnemesh.Vertices);

		return;// mesh is build, nothing to do anymore.
	}// if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_Extrusion)

	//
	// Primitives sets
	//
	if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_PointSet)
	{
		CX3DImporter_NodeElement_Set& tnemesh = *((CX3DImporter_NodeElement_Set*)&pNodeElement);// create alias for convenience

		// at first search for <Coordinate> node and create mesh.
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
			{
				std::vector<aiVector3D> vec_copy;

				vec_copy.reserve(((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value.size());
				for(std::list<aiVector3D>::const_iterator it = ((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value.begin();
					it != ((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value.end(); it++)
				{
					vec_copy.push_back(*it);
				}

				*pMesh = StandardShapes::MakeMesh(vec_copy, 1);
			}
		}

		// copy additional information from children
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			ai_assert(*pMesh);
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Color)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_Color*)*ch_it)->Value, true);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_ColorRGBA)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_ColorRGBA*)*ch_it)->Value, true);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
				{} // skip because already read when mesh created.
			else
				throw DeadlyImportError("Postprocess_BuildMesh. Unknown child of PointSet: " + to_string((*ch_it)->Type) + ".");
		}// for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)

		return;// mesh is build, nothing to do anymore.
	}// if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_PointSet)

	if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_LineSet)
	{
		CX3DImporter_NodeElement_Set& tnemesh = *((CX3DImporter_NodeElement_Set*)&pNodeElement);// create alias for convenience

		// at first search for <Coordinate> node and create mesh.
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
			{
				*pMesh = GeometryHelper_MakeMesh(tnemesh.CoordIndex, ((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value);
			}
		}

		// copy additional information from children
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			ai_assert(*pMesh);
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Color)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_Color*)*ch_it)->Value, true);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_ColorRGBA)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_ColorRGBA*)*ch_it)->Value, true);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
				{} // skip because already read when mesh created.
			else
				throw DeadlyImportError("Postprocess_BuildMesh. Unknown child of LineSet: " + to_string((*ch_it)->Type) + ".");
		}// for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)

		return;// mesh is build, nothing to do anymore.
	}// if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_LineSet)

	if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_TriangleFanSet)
	{
		CX3DImporter_NodeElement_Set& tnemesh = *((CX3DImporter_NodeElement_Set*)&pNodeElement);// create alias for convenience

		// at first search for <Coordinate> node and create mesh.
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
			{
				*pMesh = GeometryHelper_MakeMesh(tnemesh.CoordIndex, ((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value);
			}
		}

		// copy additional information from children
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			if ( nullptr == *pMesh ) {
				break;
			}
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Color)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_Color*)*ch_it)->Value,tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_ColorRGBA)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_ColorRGBA*)*ch_it)->Value, tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
				{} // skip because already read when mesh created.
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Normal)
				MeshGeometry_AddNormal(**pMesh, tnemesh.CoordIndex, tnemesh.NormalIndex, ((CX3DImporter_NodeElement_Normal*)*ch_it)->Value,
										tnemesh.NormalPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_TextureCoordinate)
				MeshGeometry_AddTexCoord(**pMesh, tnemesh.CoordIndex, tnemesh.TexCoordIndex, ((CX3DImporter_NodeElement_TextureCoordinate*)*ch_it)->Value);
			else
				throw DeadlyImportError("Postprocess_BuildMesh. Unknown child of TrianlgeFanSet: " + to_string((*ch_it)->Type) + ".");
		}// for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)

		return;// mesh is build, nothing to do anymore.
	}// if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_TriangleFanSet)

	if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_TriangleSet)
	{
		CX3DImporter_NodeElement_Set& tnemesh = *((CX3DImporter_NodeElement_Set*)&pNodeElement);// create alias for convenience

		// at first search for <Coordinate> node and create mesh.
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
			{
				std::vector<aiVector3D> vec_copy;

				vec_copy.reserve(((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value.size());
				for(std::list<aiVector3D>::const_iterator it = ((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value.begin();
					it != ((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value.end(); it++)
				{
					vec_copy.push_back(*it);
				}

				*pMesh = StandardShapes::MakeMesh(vec_copy, 3);
			}
		}

		// copy additional information from children
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			ai_assert(*pMesh);
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Color)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_Color*)*ch_it)->Value, tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_ColorRGBA)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_ColorRGBA*)*ch_it)->Value, tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
				{} // skip because already read when mesh created.
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Normal)
				MeshGeometry_AddNormal(**pMesh, tnemesh.CoordIndex, tnemesh.NormalIndex, ((CX3DImporter_NodeElement_Normal*)*ch_it)->Value,
										tnemesh.NormalPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_TextureCoordinate)
				MeshGeometry_AddTexCoord(**pMesh, tnemesh.CoordIndex, tnemesh.TexCoordIndex, ((CX3DImporter_NodeElement_TextureCoordinate*)*ch_it)->Value);
			else
				throw DeadlyImportError("Postprocess_BuildMesh. Unknown child of TrianlgeSet: " + to_string((*ch_it)->Type) + ".");
		}// for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)

		return;// mesh is build, nothing to do anymore.
	}// if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_TriangleSet)

	if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_TriangleStripSet)
	{
		CX3DImporter_NodeElement_Set& tnemesh = *((CX3DImporter_NodeElement_Set*)&pNodeElement);// create alias for convenience

		// at first search for <Coordinate> node and create mesh.
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
			{
				*pMesh = GeometryHelper_MakeMesh(tnemesh.CoordIndex, ((CX3DImporter_NodeElement_Coordinate*)*ch_it)->Value);
			}
		}

		// copy additional information from children
		for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)
		{
			ai_assert(*pMesh);
			if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Color)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_Color*)*ch_it)->Value, tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_ColorRGBA)
				MeshGeometry_AddColor(**pMesh, ((CX3DImporter_NodeElement_ColorRGBA*)*ch_it)->Value, tnemesh.ColorPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Coordinate)
				{} // skip because already read when mesh created.
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_Normal)
				MeshGeometry_AddNormal(**pMesh, tnemesh.CoordIndex, tnemesh.NormalIndex, ((CX3DImporter_NodeElement_Normal*)*ch_it)->Value,
										tnemesh.NormalPerVertex);
			else if((*ch_it)->Type == CX3DImporter_NodeElement::ENET_TextureCoordinate)
				MeshGeometry_AddTexCoord(**pMesh, tnemesh.CoordIndex, tnemesh.TexCoordIndex, ((CX3DImporter_NodeElement_TextureCoordinate*)*ch_it)->Value);
			else
				throw DeadlyImportError("Postprocess_BuildMesh. Unknown child of TriangleStripSet: " + to_string((*ch_it)->Type) + ".");
		}// for(std::list<CX3DImporter_NodeElement*>::iterator ch_it = tnemesh.Child.begin(); ch_it != tnemesh.Child.end(); ch_it++)

		return;// mesh is build, nothing to do anymore.
	}// if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_TriangleStripSet)

	throw DeadlyImportError("Postprocess_BuildMesh. Unknown mesh type: " + to_string(pNodeElement.Type) + ".");
}

void X3DImporter::Postprocess_BuildNode(const CX3DImporter_NodeElement& pNodeElement, aiNode& pSceneNode, std::list<aiMesh*>& pSceneMeshList,
										std::list<aiMaterial*>& pSceneMaterialList, std::list<aiLight*>& pSceneLightList) const
{
    std::list<CX3DImporter_NodeElement*>::const_iterator chit_begin = pNodeElement.Child.begin();
    std::list<CX3DImporter_NodeElement*>::const_iterator chit_end = pNodeElement.Child.end();
    std::list<aiNode*> SceneNode_Child;
    std::list<unsigned int> SceneNode_Mesh;

	// At first read all metadata
	Postprocess_CollectMetadata(pNodeElement, pSceneNode);
	// check if we have deal with grouping node. Which can contain transformation or switch
	if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_Group)
	{
		const CX3DImporter_NodeElement_Group& tne_group = *((CX3DImporter_NodeElement_Group*)&pNodeElement);// create alias for convenience

		pSceneNode.mTransformation = tne_group.Transformation;
		if(tne_group.UseChoice)
		{
			// If Choice is less than zero or greater than the number of nodes in the children field, nothing is chosen.
			if((tne_group.Choice < 0) || ((size_t)tne_group.Choice >= pNodeElement.Child.size()))
			{
				chit_begin = pNodeElement.Child.end();
				chit_end = pNodeElement.Child.end();
			}
			else
			{
				for(size_t i = 0; i < (size_t)tne_group.Choice; i++) chit_begin++;// forward iterator to chosen node.

				chit_end = chit_begin;
				chit_end++;// point end iterator to next element after chosen node.
			}
		}// if(tne_group.UseChoice)
	}// if(pNodeElement.Type == CX3DImporter_NodeElement::ENET_Group)

	// Reserve memory for fast access and check children.
	for(std::list<CX3DImporter_NodeElement*>::const_iterator it = chit_begin; it != chit_end; it++)
	{// in this loop we do not read metadata because it's already read at begin.
		if((*it)->Type == CX3DImporter_NodeElement::ENET_Group)
		{
			// if child is group then create new node and do recursive call.
			aiNode* new_node = new aiNode;

			new_node->mName = (*it)->ID;
			new_node->mParent = &pSceneNode;
			SceneNode_Child.push_back(new_node);
			Postprocess_BuildNode(**it, *new_node, pSceneMeshList, pSceneMaterialList, pSceneLightList);
		}
		else if((*it)->Type == CX3DImporter_NodeElement::ENET_Shape)
		{
			// shape can contain only one geometry and one appearance nodes.
			Postprocess_BuildShape(*((CX3DImporter_NodeElement_Shape*)*it), SceneNode_Mesh, pSceneMeshList, pSceneMaterialList);
		}
		else if(((*it)->Type == CX3DImporter_NodeElement::ENET_DirectionalLight) || ((*it)->Type == CX3DImporter_NodeElement::ENET_PointLight) ||
				((*it)->Type == CX3DImporter_NodeElement::ENET_SpotLight))
		{
			Postprocess_BuildLight(*((CX3DImporter_NodeElement_Light*)*it), pSceneLightList);
		}
		else if(!PostprocessHelper_ElementIsMetadata((*it)->Type))// skip metadata
		{
			throw DeadlyImportError("Postprocess_BuildNode. Unknown type: " + to_string((*it)->Type) + ".");
		}
	}// for(std::list<CX3DImporter_NodeElement*>::const_iterator it = chit_begin; it != chit_end; it++)

	// copy data about children and meshes to aiNode.
	if(SceneNode_Child.size() > 0)
	{
		std::list<aiNode*>::const_iterator it = SceneNode_Child.begin();

		pSceneNode.mNumChildren = static_cast<unsigned int>(SceneNode_Child.size());
		pSceneNode.mChildren = new aiNode*[pSceneNode.mNumChildren];
		for(size_t i = 0; i < pSceneNode.mNumChildren; i++) pSceneNode.mChildren[i] = *it++;
	}

	if(SceneNode_Mesh.size() > 0)
	{
		std::list<unsigned int>::const_iterator it = SceneNode_Mesh.begin();

		pSceneNode.mNumMeshes = static_cast<unsigned int>(SceneNode_Mesh.size());
		pSceneNode.mMeshes = new unsigned int[pSceneNode.mNumMeshes];
		for(size_t i = 0; i < pSceneNode.mNumMeshes; i++) pSceneNode.mMeshes[i] = *it++;
	}

	// that's all. return to previous deals
}

void X3DImporter::Postprocess_BuildShape(const CX3DImporter_NodeElement_Shape& pShapeNodeElement, std::list<unsigned int>& pNodeMeshInd,
							std::list<aiMesh*>& pSceneMeshList, std::list<aiMaterial*>& pSceneMaterialList) const
{
    aiMaterial* tmat = nullptr;
    aiMesh* tmesh = nullptr;
    CX3DImporter_NodeElement::EType mesh_type = CX3DImporter_NodeElement::ENET_Invalid;
    unsigned int mat_ind = 0;

	for(std::list<CX3DImporter_NodeElement*>::const_iterator it = pShapeNodeElement.Child.begin(); it != pShapeNodeElement.Child.end(); it++)
	{
		if(PostprocessHelper_ElementIsMesh((*it)->Type))
		{
			Postprocess_BuildMesh(**it, &tmesh);
			if(tmesh != nullptr)
			{
				// if mesh successfully built then add data about it to arrays
				pNodeMeshInd.push_back(static_cast<unsigned int>(pSceneMeshList.size()));
				pSceneMeshList.push_back(tmesh);
				// keep mesh type. Need above for texture coordinate generation.
				mesh_type = (*it)->Type;
			}
		}
		else if((*it)->Type == CX3DImporter_NodeElement::ENET_Appearance)
		{
			Postprocess_BuildMaterial(**it, &tmat);
			if(tmat != nullptr)
			{
				// if material successfully built then add data about it to array
				mat_ind = static_cast<unsigned int>(pSceneMaterialList.size());
				pSceneMaterialList.push_back(tmat);
			}
		}
	}// for(std::list<CX3DImporter_NodeElement*>::const_iterator it = pShapeNodeElement.Child.begin(); it != pShapeNodeElement.Child.end(); it++)

	// associate read material with read mesh.
	if((tmesh != nullptr) && (tmat != nullptr))
	{
		tmesh->mMaterialIndex = mat_ind;
		// Check texture mapping. If material has texture but mesh has no texture coordinate then try to ask Assimp to generate texture coordinates.
		if((tmat->GetTextureCount(aiTextureType_DIFFUSE) != 0) && !tmesh->HasTextureCoords(0))
		{
			int32_t tm;
			aiVector3D tvec3;

			switch(mesh_type)
			{
				case CX3DImporter_NodeElement::ENET_Box:
					tm = aiTextureMapping_BOX;
					break;
				case CX3DImporter_NodeElement::ENET_Cone:
				case CX3DImporter_NodeElement::ENET_Cylinder:
					tm = aiTextureMapping_CYLINDER;
					break;
				case CX3DImporter_NodeElement::ENET_Sphere:
					tm = aiTextureMapping_SPHERE;
					break;
				default:
					tm = aiTextureMapping_PLANE;
					break;
			}// switch(mesh_type)

			tmat->AddProperty(&tm, 1, AI_MATKEY_MAPPING_DIFFUSE(0));
		}// if((tmat->GetTextureCount(aiTextureType_DIFFUSE) != 0) && !tmesh->HasTextureCoords(0))
	}// if((tmesh != nullptr) && (tmat != nullptr))
}

void X3DImporter::Postprocess_CollectMetadata(const CX3DImporter_NodeElement& pNodeElement, aiNode& pSceneNode) const
{
    std::list<CX3DImporter_NodeElement*> meta_list;
    size_t meta_idx;

	PostprocessHelper_CollectMetadata(pNodeElement, meta_list);// find metadata in current node element.
	if ( !meta_list.empty() )
	{
        if ( pSceneNode.mMetaData != nullptr ) {
            throw DeadlyImportError( "Postprocess. MetaData member in node are not nullptr. Something went wrong." );
        }

		// copy collected metadata to output node.
        pSceneNode.mMetaData = aiMetadata::Alloc( static_cast<unsigned int>(meta_list.size()) );
		meta_idx = 0;
		for(std::list<CX3DImporter_NodeElement*>::const_iterator it = meta_list.begin(); it != meta_list.end(); it++, meta_idx++)
		{
			CX3DImporter_NodeElement_Meta* cur_meta = (CX3DImporter_NodeElement_Meta*)*it;

			// due to limitations we can add only first element of value list.
			// Add an element according to its type.
			if((*it)->Type == CX3DImporter_NodeElement::ENET_MetaBoolean)
			{
				if(((CX3DImporter_NodeElement_MetaBoolean*)cur_meta)->Value.size() > 0)
					pSceneNode.mMetaData->Set(static_cast<unsigned int>(meta_idx), cur_meta->Name, *(((CX3DImporter_NodeElement_MetaBoolean*)cur_meta)->Value.begin()));
			}
			else if((*it)->Type == CX3DImporter_NodeElement::ENET_MetaDouble)
			{
				if(((CX3DImporter_NodeElement_MetaDouble*)cur_meta)->Value.size() > 0)
					pSceneNode.mMetaData->Set(static_cast<unsigned int>(meta_idx), cur_meta->Name, (float)*(((CX3DImporter_NodeElement_MetaDouble*)cur_meta)->Value.begin()));
			}
			else if((*it)->Type == CX3DImporter_NodeElement::ENET_MetaFloat)
			{
				if(((CX3DImporter_NodeElement_MetaFloat*)cur_meta)->Value.size() > 0)
					pSceneNode.mMetaData->Set(static_cast<unsigned int>(meta_idx), cur_meta->Name, *(((CX3DImporter_NodeElement_MetaFloat*)cur_meta)->Value.begin()));
			}
			else if((*it)->Type == CX3DImporter_NodeElement::ENET_MetaInteger)
			{
				if(((CX3DImporter_NodeElement_MetaInteger*)cur_meta)->Value.size() > 0)
					pSceneNode.mMetaData->Set(static_cast<unsigned int>(meta_idx), cur_meta->Name, *(((CX3DImporter_NodeElement_MetaInteger*)cur_meta)->Value.begin()));
			}
			else if((*it)->Type == CX3DImporter_NodeElement::ENET_MetaString)
			{
				if(((CX3DImporter_NodeElement_MetaString*)cur_meta)->Value.size() > 0)
				{
					aiString tstr(((CX3DImporter_NodeElement_MetaString*)cur_meta)->Value.begin()->data());

					pSceneNode.mMetaData->Set(static_cast<unsigned int>(meta_idx), cur_meta->Name, tstr);
				}
			}
			else
			{
				throw DeadlyImportError("Postprocess. Unknown metadata type.");
			}// if((*it)->Type == CX3DImporter_NodeElement::ENET_Meta*) else
		}// for(std::list<CX3DImporter_NodeElement*>::const_iterator it = meta_list.begin(); it != meta_list.end(); it++)
	}// if( !meta_list.empty() )
}

}// namespace Assimp

#endif // !ASSIMP_BUILD_NO_X3D_IMPORTER
