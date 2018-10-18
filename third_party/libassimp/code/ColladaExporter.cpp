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

#ifndef ASSIMP_BUILD_NO_EXPORT
#ifndef ASSIMP_BUILD_NO_COLLADA_EXPORTER

#include "ColladaExporter.h"
#include <assimp/Bitmap.h>
#include <assimp/fast_atof.h>
#include <assimp/SceneCombiner.h>
#include <assimp/StringUtils.h>
#include <assimp/XMLTools.h>
#include <assimp/DefaultIOSystem.h>
#include <assimp/IOSystem.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>

#include <assimp/Exceptional.h>

#include <memory>
#include <ctime>
#include <set>
#include <vector>
#include <iostream>

using namespace Assimp;

namespace Assimp
{

// ------------------------------------------------------------------------------------------------
// Worker function for exporting a scene to Collada. Prototyped and registered in Exporter.cpp
void ExportSceneCollada(const char* pFile, IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* /*pProperties*/)
{
    std::string path = DefaultIOSystem::absolutePath(std::string(pFile));
    std::string file = DefaultIOSystem::completeBaseName(std::string(pFile));

    // invoke the exporter
    ColladaExporter iDoTheExportThing( pScene, pIOSystem, path, file);
    
    if (iDoTheExportThing.mOutput.fail()) {
        throw DeadlyExportError("output data creation failed. Most likely the file became too large: " + std::string(pFile));
    }

    // we're still here - export successfully completed. Write result to the given IOSYstem
    std::unique_ptr<IOStream> outfile (pIOSystem->Open(pFile,"wt"));
    if(outfile == NULL) {
        throw DeadlyExportError("could not open output .dae file: " + std::string(pFile));
    }

    // XXX maybe use a small wrapper around IOStream that behaves like std::stringstream in order to avoid the extra copy.
    outfile->Write( iDoTheExportThing.mOutput.str().c_str(), static_cast<size_t>(iDoTheExportThing.mOutput.tellp()),1);
}

} // end of namespace Assimp



// ------------------------------------------------------------------------------------------------
// Constructor for a specific scene to export
ColladaExporter::ColladaExporter( const aiScene* pScene, IOSystem* pIOSystem, const std::string& path, const std::string& file) : mIOSystem(pIOSystem), mPath(path), mFile(file)
{
    // make sure that all formatting happens using the standard, C locale and not the user's current locale
    mOutput.imbue( std::locale("C") );
    mOutput.precision(16);

    mScene = pScene;
    mSceneOwned = false;

    // set up strings
    endstr = "\n";

    // start writing the file
    WriteFile();
}

// ------------------------------------------------------------------------------------------------
// Destructor
ColladaExporter::~ColladaExporter()
{
    if(mSceneOwned) {
        delete mScene;
    }
}

// ------------------------------------------------------------------------------------------------
// Starts writing the contents
void ColladaExporter::WriteFile()
{
    // write the DTD
    mOutput << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" << endstr;
    // COLLADA element start
    mOutput << "<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">" << endstr;
    PushTag();

    WriteTextures();
    WriteHeader();

    WriteCamerasLibrary();
    WriteLightsLibrary();
    WriteMaterials();
    WriteGeometryLibrary();
    WriteControllerLibrary();

    WriteSceneLibrary();
	
	// customized, Writes the animation library
	WriteAnimationsLibrary();

    // useless Collada fu at the end, just in case we haven't had enough indirections, yet.
    mOutput << startstr << "<scene>" << endstr;
    PushTag();
    mOutput << startstr << "<instance_visual_scene url=\"#" + XMLEscape(mScene->mRootNode->mName.C_Str()) + "\" />" << endstr;
    PopTag();
    mOutput << startstr << "</scene>" << endstr;
    PopTag();
    mOutput << "</COLLADA>" << endstr;
}

// ------------------------------------------------------------------------------------------------
// Writes the asset header
void ColladaExporter::WriteHeader()
{
    static const ai_real epsilon = ai_real( 0.00001 );
    static const aiQuaternion x_rot(aiMatrix3x3(
        0, -1,  0,
        1,  0,  0,
        0,  0,  1));
    static const aiQuaternion y_rot(aiMatrix3x3(
        1,  0,  0,
        0,  1,  0,
        0,  0,  1));
    static const aiQuaternion z_rot(aiMatrix3x3(
        1,  0,  0,
        0,  0,  1,
        0, -1,  0));

    static const unsigned int date_nb_chars = 20;
    char date_str[date_nb_chars];
    std::time_t date = std::time(NULL);
    std::strftime(date_str, date_nb_chars, "%Y-%m-%dT%H:%M:%S", std::localtime(&date));

    aiVector3D scaling;
    aiQuaternion rotation;
    aiVector3D position;
    mScene->mRootNode->mTransformation.Decompose(scaling, rotation, position);
    rotation.Normalize();

    bool add_root_node = false;

    ai_real scale = 1.0;
    if(std::abs(scaling.x - scaling.y) <= epsilon && std::abs(scaling.x - scaling.z) <= epsilon && std::abs(scaling.y - scaling.z) <= epsilon) {
        scale = (ai_real) ((((double) scaling.x) + ((double) scaling.y) + ((double) scaling.z)) / 3.0);
    } else {
        add_root_node = true;
    }

    std::string up_axis = "Y_UP";
    if(rotation.Equal(x_rot, epsilon)) {
        up_axis = "X_UP";
    } else if(rotation.Equal(y_rot, epsilon)) {
        up_axis = "Y_UP";
    } else if(rotation.Equal(z_rot, epsilon)) {
        up_axis = "Z_UP";
    } else {
        add_root_node = true;
    }

    if(! position.Equal(aiVector3D(0, 0, 0))) {
        add_root_node = true;
    }

    if(mScene->mRootNode->mNumChildren == 0) {
        add_root_node = true;
    }

    if(add_root_node) {
        aiScene* scene;
        SceneCombiner::CopyScene(&scene, mScene);

        aiNode* root = new aiNode("Scene");

        root->mNumChildren = 1;
        root->mChildren = new aiNode*[root->mNumChildren];

        root->mChildren[0] = scene->mRootNode;
        scene->mRootNode->mParent = root;
        scene->mRootNode = root;

        mScene = scene;
        mSceneOwned = true;

        up_axis = "Y_UP";
        scale = 1.0;
    }

    mOutput << startstr << "<asset>" << endstr;
    PushTag();
    mOutput << startstr << "<contributor>" << endstr;
    PushTag();

    aiMetadata* meta = mScene->mRootNode->mMetaData;
    aiString value;
    if (!meta || !meta->Get("Author", value))
        mOutput << startstr << "<author>" << "Assimp" << "</author>" << endstr;
    else
        mOutput << startstr << "<author>" << XMLEscape(value.C_Str()) << "</author>" << endstr;

    if (!meta || !meta->Get("AuthoringTool", value))
        mOutput << startstr << "<authoring_tool>" << "Assimp Exporter" << "</authoring_tool>" << endstr;
    else
        mOutput << startstr << "<authoring_tool>" << XMLEscape(value.C_Str()) << "</authoring_tool>" << endstr;

    //mOutput << startstr << "<author>" << mScene->author.C_Str() << "</author>" << endstr;
    //mOutput << startstr << "<authoring_tool>" << mScene->authoringTool.C_Str() << "</authoring_tool>" << endstr;

    PopTag();
    mOutput << startstr << "</contributor>" << endstr;
    mOutput << startstr << "<created>" << date_str << "</created>" << endstr;
    mOutput << startstr << "<modified>" << date_str << "</modified>" << endstr;
    mOutput << startstr << "<unit name=\"meter\" meter=\"" << scale << "\" />" << endstr;
    mOutput << startstr << "<up_axis>" << up_axis << "</up_axis>" << endstr;
    PopTag();
    mOutput << startstr << "</asset>" << endstr;
}

// ------------------------------------------------------------------------------------------------
// Write the embedded textures
void ColladaExporter::WriteTextures() {
    static const unsigned int buffer_size = 1024;
    char str[buffer_size];

    if(mScene->HasTextures()) {
        for(unsigned int i = 0; i < mScene->mNumTextures; i++) {
            // It would be great to be able to create a directory in portable standard C++, but it's not the case,
            // so we just write the textures in the current directory.

            aiTexture* texture = mScene->mTextures[i];

            ASSIMP_itoa10(str, buffer_size, i + 1);

            std::string name = mFile + "_texture_" + (i < 1000 ? "0" : "") + (i < 100 ? "0" : "") + (i < 10 ? "0" : "") + str + "." + ((const char*) texture->achFormatHint);

            std::unique_ptr<IOStream> outfile(mIOSystem->Open(mPath + name, "wb"));
            if(outfile == NULL) {
                throw DeadlyExportError("could not open output texture file: " + mPath + name);
            }

            if(texture->mHeight == 0) {
                outfile->Write((void*) texture->pcData, texture->mWidth, 1);
            } else {
                Bitmap::Save(texture, outfile.get());
            }

            outfile->Flush();

            textures.insert(std::make_pair(i, name));
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Write the embedded textures
void ColladaExporter::WriteCamerasLibrary() {
    if(mScene->HasCameras()) {

        mOutput << startstr << "<library_cameras>" << endstr;
        PushTag();

        for( size_t a = 0; a < mScene->mNumCameras; ++a)
            WriteCamera( a);

        PopTag();
        mOutput << startstr << "</library_cameras>" << endstr;

    }
}

void ColladaExporter::WriteCamera(size_t pIndex){

    const aiCamera *cam = mScene->mCameras[pIndex];
    const std::string idstrEscaped = XMLEscape(cam->mName.C_Str());

    mOutput << startstr << "<camera id=\"" << idstrEscaped << "-camera\" name=\"" << idstrEscaped << "_name\" >" << endstr;
    PushTag();
    mOutput << startstr << "<optics>" << endstr;
    PushTag();
    mOutput << startstr << "<technique_common>" << endstr;
    PushTag();
    //assimp doesn't support the import of orthographic cameras! se we write
    //always perspective
    mOutput << startstr << "<perspective>" << endstr;
    PushTag();
    mOutput << startstr << "<xfov sid=\"xfov\">"<<
                                AI_RAD_TO_DEG(cam->mHorizontalFOV)
                        <<"</xfov>" << endstr;
    mOutput << startstr << "<aspect_ratio>"
                        <<      cam->mAspect
                        << "</aspect_ratio>" << endstr;
    mOutput << startstr << "<znear sid=\"znear\">"
                        <<      cam->mClipPlaneNear
                        <<  "</znear>" << endstr;
    mOutput << startstr << "<zfar sid=\"zfar\">"
                        <<      cam->mClipPlaneFar
                        << "</zfar>" << endstr;
    PopTag();
    mOutput << startstr << "</perspective>" << endstr;
    PopTag();
    mOutput << startstr << "</technique_common>" << endstr;
    PopTag();
    mOutput << startstr << "</optics>" << endstr;
    PopTag();
    mOutput << startstr << "</camera>" << endstr;

}


// ------------------------------------------------------------------------------------------------
// Write the embedded textures
void ColladaExporter::WriteLightsLibrary() {
    if(mScene->HasLights()) {

        mOutput << startstr << "<library_lights>" << endstr;
        PushTag();

        for( size_t a = 0; a < mScene->mNumLights; ++a)
            WriteLight( a);

        PopTag();
        mOutput << startstr << "</library_lights>" << endstr;

    }
}

void ColladaExporter::WriteLight(size_t pIndex){

    const aiLight *light = mScene->mLights[pIndex];
    const std::string idstrEscaped = XMLEscape(light->mName.C_Str());

    mOutput << startstr << "<light id=\"" << idstrEscaped << "-light\" name=\""
            << idstrEscaped << "_name\" >" << endstr;
    PushTag();
    mOutput << startstr << "<technique_common>" << endstr;
    PushTag();
    switch(light->mType){
        case aiLightSource_AMBIENT:
            WriteAmbienttLight(light);
            break;
        case aiLightSource_DIRECTIONAL:
            WriteDirectionalLight(light);
            break;
        case aiLightSource_POINT:
            WritePointLight(light);
            break;
        case aiLightSource_SPOT:
            WriteSpotLight(light);
            break;
        case aiLightSource_AREA:
        case aiLightSource_UNDEFINED:
        case _aiLightSource_Force32Bit:
            break;
    }
    PopTag();
    mOutput << startstr << "</technique_common>" << endstr;

    PopTag();
    mOutput << startstr << "</light>" << endstr;

}

void ColladaExporter::WritePointLight(const aiLight *const light){
    const aiColor3D &color=  light->mColorDiffuse;
    mOutput << startstr << "<point>" << endstr;
    PushTag();
    mOutput << startstr << "<color sid=\"color\">"
                            << color.r<<" "<<color.g<<" "<<color.b
                        <<"</color>" << endstr;
    mOutput << startstr << "<constant_attenuation>"
                            << light->mAttenuationConstant
                        <<"</constant_attenuation>" << endstr;
    mOutput << startstr << "<linear_attenuation>"
                            << light->mAttenuationLinear
                        <<"</linear_attenuation>" << endstr;
    mOutput << startstr << "<quadratic_attenuation>"
                            << light->mAttenuationQuadratic
                        <<"</quadratic_attenuation>" << endstr;

    PopTag();
    mOutput << startstr << "</point>" << endstr;

}
void ColladaExporter::WriteDirectionalLight(const aiLight *const light){
    const aiColor3D &color=  light->mColorDiffuse;
    mOutput << startstr << "<directional>" << endstr;
    PushTag();
    mOutput << startstr << "<color sid=\"color\">"
                            << color.r<<" "<<color.g<<" "<<color.b
                        <<"</color>" << endstr;

    PopTag();
    mOutput << startstr << "</directional>" << endstr;

}
void ColladaExporter::WriteSpotLight(const aiLight *const light){

    const aiColor3D &color=  light->mColorDiffuse;
    mOutput << startstr << "<spot>" << endstr;
    PushTag();
    mOutput << startstr << "<color sid=\"color\">"
                            << color.r<<" "<<color.g<<" "<<color.b
                        <<"</color>" << endstr;
    mOutput << startstr << "<constant_attenuation>"
                                << light->mAttenuationConstant
                            <<"</constant_attenuation>" << endstr;
    mOutput << startstr << "<linear_attenuation>"
                            << light->mAttenuationLinear
                        <<"</linear_attenuation>" << endstr;
    mOutput << startstr << "<quadratic_attenuation>"
                            << light->mAttenuationQuadratic
                        <<"</quadratic_attenuation>" << endstr;
    /*
    out->mAngleOuterCone = AI_DEG_TO_RAD (std::acos(std::pow(0.1f,1.f/srcLight->mFalloffExponent))+
                            srcLight->mFalloffAngle);
    */

    const ai_real fallOffAngle = AI_RAD_TO_DEG(light->mAngleInnerCone);
    mOutput << startstr <<"<falloff_angle sid=\"fall_off_angle\">"
                                << fallOffAngle
                        <<"</falloff_angle>" << endstr;
    double temp = light->mAngleOuterCone-light->mAngleInnerCone;

    temp = std::cos(temp);
    temp = std::log(temp)/std::log(0.1);
    temp = 1/temp;
    mOutput << startstr << "<falloff_exponent sid=\"fall_off_exponent\">"
                            << temp
                        <<"</falloff_exponent>" << endstr;


    PopTag();
    mOutput << startstr << "</spot>" << endstr;

}

void ColladaExporter::WriteAmbienttLight(const aiLight *const light){

    const aiColor3D &color=  light->mColorAmbient;
    mOutput << startstr << "<ambient>" << endstr;
    PushTag();
    mOutput << startstr << "<color sid=\"color\">"
                            << color.r<<" "<<color.g<<" "<<color.b
                        <<"</color>" << endstr;

    PopTag();
    mOutput << startstr << "</ambient>" << endstr;
}

// ------------------------------------------------------------------------------------------------
// Reads a single surface entry from the given material keys
void ColladaExporter::ReadMaterialSurface( Surface& poSurface, const aiMaterial* pSrcMat, aiTextureType pTexture, const char* pKey, size_t pType, size_t pIndex)
{
  if( pSrcMat->GetTextureCount( pTexture) > 0 )
  {
    aiString texfile;
    unsigned int uvChannel = 0;
    pSrcMat->GetTexture( pTexture, 0, &texfile, NULL, &uvChannel);

    std::string index_str(texfile.C_Str());

    if(index_str.size() != 0 && index_str[0] == '*')
    {
        unsigned int index;

        index_str = index_str.substr(1, std::string::npos);

        try {
            index = (unsigned int) strtoul10_64(index_str.c_str());
        } catch(std::exception& error) {
            throw DeadlyExportError(error.what());
        }

        std::map<unsigned int, std::string>::const_iterator name = textures.find(index);

        if(name != textures.end()) {
            poSurface.texture = name->second;
        } else {
            throw DeadlyExportError("could not find embedded texture at index " + index_str);
        }
    } else
    {
        poSurface.texture = texfile.C_Str();
    }

    poSurface.channel = uvChannel;
    poSurface.exist = true;
  } else
  {
    if( pKey )
      poSurface.exist = pSrcMat->Get( pKey, static_cast<unsigned int>(pType), static_cast<unsigned int>(pIndex), poSurface.color) == aiReturn_SUCCESS;
  }
}

// ------------------------------------------------------------------------------------------------
// Reimplementation of isalnum(,C locale), because AppVeyor does not see standard version.
static bool isalnum_C(char c)
{
  return ( nullptr != strchr("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",c) );
}

// ------------------------------------------------------------------------------------------------
// Writes an image entry for the given surface
void ColladaExporter::WriteImageEntry( const Surface& pSurface, const std::string& pNameAdd)
{
  if( !pSurface.texture.empty() )
  {
    mOutput << startstr << "<image id=\"" << XMLEscape(pNameAdd) << "\">" << endstr;
    PushTag();
    mOutput << startstr << "<init_from>";

    // URL encode image file name first, then XML encode on top
    std::stringstream imageUrlEncoded;
    for( std::string::const_iterator it = pSurface.texture.begin(); it != pSurface.texture.end(); ++it )
    {
      if( isalnum_C( (unsigned char) *it) || *it == ':' || *it == '_' || *it == '-' || *it == '.' || *it == '/' || *it == '\\' )
        imageUrlEncoded << *it;
      else
        imageUrlEncoded << '%' << std::hex << size_t( (unsigned char) *it) << std::dec;
    }
    mOutput << XMLEscape(imageUrlEncoded.str());
    mOutput << "</init_from>" << endstr;
    PopTag();
    mOutput << startstr << "</image>" << endstr;
  }
}

// ------------------------------------------------------------------------------------------------
// Writes a color-or-texture entry into an effect definition
void ColladaExporter::WriteTextureColorEntry( const Surface& pSurface, const std::string& pTypeName, const std::string& pImageName)
{
  if(pSurface.exist) {
    mOutput << startstr << "<" << pTypeName << ">" << endstr;
    PushTag();
    if( pSurface.texture.empty() )
    {
      mOutput << startstr << "<color sid=\"" << pTypeName << "\">" << pSurface.color.r << "   " << pSurface.color.g << "   " << pSurface.color.b << "   " << pSurface.color.a << "</color>" << endstr;
    }
    else
    {
      mOutput << startstr << "<texture texture=\"" << XMLEscape(pImageName) << "\" texcoord=\"CHANNEL" << pSurface.channel << "\" />" << endstr;
    }
    PopTag();
    mOutput << startstr << "</" << pTypeName << ">" << endstr;
  }
}

// ------------------------------------------------------------------------------------------------
// Writes the two parameters necessary for referencing a texture in an effect entry
void ColladaExporter::WriteTextureParamEntry( const Surface& pSurface, const std::string& pTypeName, const std::string& pMatName)
{
  // if surface is a texture, write out the sampler and the surface parameters necessary to reference the texture
  if( !pSurface.texture.empty() )
  {
    mOutput << startstr << "<newparam sid=\"" << XMLEscape(pMatName) << "-" << pTypeName << "-surface\">" << endstr;
    PushTag();
    mOutput << startstr << "<surface type=\"2D\">" << endstr;
    PushTag();
    mOutput << startstr << "<init_from>" << XMLEscape(pMatName) << "-" << pTypeName << "-image</init_from>" << endstr;
    PopTag();
    mOutput << startstr << "</surface>" << endstr;
    PopTag();
    mOutput << startstr << "</newparam>" << endstr;

    mOutput << startstr << "<newparam sid=\"" << XMLEscape(pMatName) << "-" << pTypeName << "-sampler\">" << endstr;
    PushTag();
    mOutput << startstr << "<sampler2D>" << endstr;
    PushTag();
    mOutput << startstr << "<source>" << XMLEscape(pMatName) << "-" << pTypeName << "-surface</source>" << endstr;
    PopTag();
    mOutput << startstr << "</sampler2D>" << endstr;
    PopTag();
    mOutput << startstr << "</newparam>" << endstr;
  }
}

// ------------------------------------------------------------------------------------------------
// Writes a scalar property
void ColladaExporter::WriteFloatEntry( const Property& pProperty, const std::string& pTypeName)
{
    if(pProperty.exist) {
        mOutput << startstr << "<" << pTypeName << ">" << endstr;
        PushTag();
        mOutput << startstr << "<float sid=\"" << pTypeName << "\">" << pProperty.value << "</float>" << endstr;
        PopTag();
        mOutput << startstr << "</" << pTypeName << ">" << endstr;
    }
}

// ------------------------------------------------------------------------------------------------
// Writes the material setup
void ColladaExporter::WriteMaterials()
{
  materials.resize( mScene->mNumMaterials);

  /// collect all materials from the scene
  size_t numTextures = 0;
  for( size_t a = 0; a < mScene->mNumMaterials; ++a )
  {
    const aiMaterial* mat = mScene->mMaterials[a];

    aiString name;
    if( mat->Get( AI_MATKEY_NAME, name) != aiReturn_SUCCESS ) {
      name = "mat";
      materials[a].name = std::string( "m") + to_string(a) + name.C_Str();
    } else {
      // try to use the material's name if no other material has already taken it, else append #
      std::string testName = name.C_Str();
      size_t materialCountWithThisName = 0;
      for( size_t i = 0; i < a; i ++ ) {
        if( materials[i].name == testName ) {
          materialCountWithThisName ++;
        }
      }
      if( materialCountWithThisName == 0 ) {
        materials[a].name = name.C_Str();
      } else {
        materials[a].name = std::string(name.C_Str()) + to_string(materialCountWithThisName);
      }
    }
    for( std::string::iterator it = materials[a].name.begin(); it != materials[a].name.end(); ++it ) {
      if( !isalnum_C( *it ) ) {
        *it = '_';
      }
    }

    aiShadingMode shading = aiShadingMode_Flat;
    materials[a].shading_model = "phong";
    if(mat->Get( AI_MATKEY_SHADING_MODEL, shading) == aiReturn_SUCCESS) {
        if(shading == aiShadingMode_Phong) {
            materials[a].shading_model = "phong";
        } else if(shading == aiShadingMode_Blinn) {
            materials[a].shading_model = "blinn";
        } else if(shading == aiShadingMode_NoShading) {
            materials[a].shading_model = "constant";
        } else if(shading == aiShadingMode_Gouraud) {
            materials[a].shading_model = "lambert";
        }
    }

    ReadMaterialSurface( materials[a].ambient, mat, aiTextureType_AMBIENT, AI_MATKEY_COLOR_AMBIENT);
    if( !materials[a].ambient.texture.empty() ) numTextures++;
    ReadMaterialSurface( materials[a].diffuse, mat, aiTextureType_DIFFUSE, AI_MATKEY_COLOR_DIFFUSE);
    if( !materials[a].diffuse.texture.empty() ) numTextures++;
    ReadMaterialSurface( materials[a].specular, mat, aiTextureType_SPECULAR, AI_MATKEY_COLOR_SPECULAR);
    if( !materials[a].specular.texture.empty() ) numTextures++;
    ReadMaterialSurface( materials[a].emissive, mat, aiTextureType_EMISSIVE, AI_MATKEY_COLOR_EMISSIVE);
    if( !materials[a].emissive.texture.empty() ) numTextures++;
    ReadMaterialSurface( materials[a].reflective, mat, aiTextureType_REFLECTION, AI_MATKEY_COLOR_REFLECTIVE);
    if( !materials[a].reflective.texture.empty() ) numTextures++;
    ReadMaterialSurface( materials[a].transparent, mat, aiTextureType_OPACITY, AI_MATKEY_COLOR_TRANSPARENT);
    if( !materials[a].transparent.texture.empty() ) numTextures++;
    ReadMaterialSurface( materials[a].normal, mat, aiTextureType_NORMALS, NULL, 0, 0);
    if( !materials[a].normal.texture.empty() ) numTextures++;

    materials[a].shininess.exist = mat->Get( AI_MATKEY_SHININESS, materials[a].shininess.value) == aiReturn_SUCCESS;
    materials[a].transparency.exist = mat->Get( AI_MATKEY_OPACITY, materials[a].transparency.value) == aiReturn_SUCCESS;
    materials[a].index_refraction.exist = mat->Get( AI_MATKEY_REFRACTI, materials[a].index_refraction.value) == aiReturn_SUCCESS;
  }

  // output textures if present
  if( numTextures > 0 )
  {
    mOutput << startstr << "<library_images>" << endstr;
    PushTag();
    for( std::vector<Material>::const_iterator it = materials.begin(); it != materials.end(); ++it )
    {
      const Material& mat = *it;
      WriteImageEntry( mat.ambient, mat.name + "-ambient-image");
      WriteImageEntry( mat.diffuse, mat.name + "-diffuse-image");
      WriteImageEntry( mat.specular, mat.name + "-specular-image");
      WriteImageEntry( mat.emissive, mat.name + "-emission-image");
      WriteImageEntry( mat.reflective, mat.name + "-reflective-image");
      WriteImageEntry( mat.transparent, mat.name + "-transparent-image");
      WriteImageEntry( mat.normal, mat.name + "-normal-image");
    }
    PopTag();
    mOutput << startstr << "</library_images>" << endstr;
  }

  // output effects - those are the actual carriers of information
  if( !materials.empty() )
  {
    mOutput << startstr << "<library_effects>" << endstr;
    PushTag();
    for( std::vector<Material>::const_iterator it = materials.begin(); it != materials.end(); ++it )
    {
      const Material& mat = *it;
      // this is so ridiculous it must be right
      mOutput << startstr << "<effect id=\"" << XMLEscape(mat.name) << "-fx\" name=\"" << XMLEscape(mat.name) << "\">" << endstr;
      PushTag();
      mOutput << startstr << "<profile_COMMON>" << endstr;
      PushTag();

      // write sampler- and surface params for the texture entries
      WriteTextureParamEntry( mat.emissive, "emission", mat.name);
      WriteTextureParamEntry( mat.ambient, "ambient", mat.name);
      WriteTextureParamEntry( mat.diffuse, "diffuse", mat.name);
      WriteTextureParamEntry( mat.specular, "specular", mat.name);
      WriteTextureParamEntry( mat.reflective, "reflective", mat.name);
      WriteTextureParamEntry( mat.transparent, "transparent", mat.name);
      WriteTextureParamEntry( mat.normal, "normal", mat.name);

      mOutput << startstr << "<technique sid=\"standard\">" << endstr;
      PushTag();
      mOutput << startstr << "<" << mat.shading_model << ">" << endstr;
      PushTag();

      WriteTextureColorEntry( mat.emissive, "emission", mat.name + "-emission-sampler");
      WriteTextureColorEntry( mat.ambient, "ambient", mat.name + "-ambient-sampler");
      WriteTextureColorEntry( mat.diffuse, "diffuse", mat.name + "-diffuse-sampler");
      WriteTextureColorEntry( mat.specular, "specular", mat.name + "-specular-sampler");
      WriteFloatEntry(mat.shininess, "shininess");
      WriteTextureColorEntry( mat.reflective, "reflective", mat.name + "-reflective-sampler");
      WriteTextureColorEntry( mat.transparent, "transparent", mat.name + "-transparent-sampler");
      WriteFloatEntry(mat.transparency, "transparency");
      WriteFloatEntry(mat.index_refraction, "index_of_refraction");

      if(! mat.normal.texture.empty()) {
        WriteTextureColorEntry( mat.normal, "bump", mat.name + "-normal-sampler");
      }

      PopTag();
      mOutput << startstr << "</" << mat.shading_model << ">" << endstr;
      PopTag();
      mOutput << startstr << "</technique>" << endstr;
      PopTag();
      mOutput << startstr << "</profile_COMMON>" << endstr;
      PopTag();
      mOutput << startstr << "</effect>" << endstr;
    }
    PopTag();
    mOutput << startstr << "</library_effects>" << endstr;

    // write materials - they're just effect references
    mOutput << startstr << "<library_materials>" << endstr;
    PushTag();
    for( std::vector<Material>::const_iterator it = materials.begin(); it != materials.end(); ++it )
    {
      const Material& mat = *it;
      mOutput << startstr << "<material id=\"" << XMLEscape(mat.name) << "\" name=\"" << mat.name << "\">" << endstr;
      PushTag();
      mOutput << startstr << "<instance_effect url=\"#" << XMLEscape(mat.name) << "-fx\"/>" << endstr;
      PopTag();
      mOutput << startstr << "</material>" << endstr;
    }
    PopTag();
    mOutput << startstr << "</library_materials>" << endstr;
  }
}

// ------------------------------------------------------------------------------------------------
// Writes the controller library
void ColladaExporter::WriteControllerLibrary()
{
    mOutput << startstr << "<library_controllers>" << endstr;
    PushTag();
    
    for( size_t a = 0; a < mScene->mNumMeshes; ++a)
        WriteController( a);

    PopTag();
    mOutput << startstr << "</library_controllers>" << endstr;
}

// ------------------------------------------------------------------------------------------------
// Writes a skin controller of the given mesh
void ColladaExporter::WriteController( size_t pIndex)
{
    const aiMesh* mesh = mScene->mMeshes[pIndex];
    const std::string idstr = GetMeshId( pIndex);
    const std::string idstrEscaped = XMLEscape(idstr);

    if ( mesh->mNumFaces == 0 || mesh->mNumVertices == 0 )
        return;

    if ( mesh->mNumBones == 0 )
        return;

    mOutput << startstr << "<controller id=\"" << idstrEscaped << "-skin\" ";
    mOutput << "name=\"skinCluster" << pIndex << "\">"<< endstr;
    PushTag();

    mOutput << startstr << "<skin source=\"#" << idstrEscaped << "\">" << endstr;
    PushTag();

    // bind pose matrix
    mOutput << startstr << "<bind_shape_matrix>" << endstr;
    PushTag();

    // I think it is identity in general cases.
    aiMatrix4x4 mat;
    mOutput << startstr << mat.a1 << " " << mat.a2 << " " << mat.a3 << " " << mat.a4 << endstr;
    mOutput << startstr << mat.b1 << " " << mat.b2 << " " << mat.b3 << " " << mat.b4 << endstr;
    mOutput << startstr << mat.c1 << " " << mat.c2 << " " << mat.c3 << " " << mat.c4 << endstr;
    mOutput << startstr << mat.d1 << " " << mat.d2 << " " << mat.d3 << " " << mat.d4 << endstr;

    PopTag();
    mOutput << startstr << "</bind_shape_matrix>" << endstr;

    mOutput << startstr << "<source id=\"" << idstrEscaped << "-skin-joints\" name=\"" << idstrEscaped << "-skin-joints\">" << endstr;
    PushTag();

    mOutput << startstr << "<Name_array id=\"" << idstrEscaped << "-skin-joints-array\" count=\"" << mesh->mNumBones << "\">";

    for( size_t i = 0; i < mesh->mNumBones; ++i )
        mOutput << XMLEscape(mesh->mBones[i]->mName.C_Str()) << " ";

    mOutput << "</Name_array>" << endstr;

    mOutput << startstr << "<technique_common>" << endstr;
    PushTag();
    
    mOutput << startstr << "<accessor source=\"#" << idstrEscaped << "-skin-joints-array\" count=\"" << mesh->mNumBones << "\" stride=\"" << 1 << "\">" << endstr;
    PushTag();

    mOutput << startstr << "<param name=\"JOINT\" type=\"Name\"></param>" << endstr;

    PopTag();
    mOutput << startstr << "</accessor>" << endstr;

    PopTag();
    mOutput << startstr << "</technique_common>" << endstr;

    PopTag();
    mOutput << startstr << "</source>" << endstr;

    std::vector<ai_real> bind_poses;
    bind_poses.reserve(mesh->mNumBones * 16);
    for(unsigned int i = 0; i < mesh->mNumBones; ++i)
        for( unsigned int j = 0; j < 4; ++j)
            bind_poses.insert(bind_poses.end(), mesh->mBones[i]->mOffsetMatrix[j], mesh->mBones[i]->mOffsetMatrix[j] + 4);

    WriteFloatArray( idstr + "-skin-bind_poses", FloatType_Mat4x4, (const ai_real*) bind_poses.data(), bind_poses.size() / 16);

    bind_poses.clear();
    
    std::vector<ai_real> skin_weights;
    skin_weights.reserve(mesh->mNumVertices * mesh->mNumBones);
    for( size_t i = 0; i < mesh->mNumBones; ++i)
        for( size_t j = 0; j < mesh->mBones[i]->mNumWeights; ++j)
            skin_weights.push_back(mesh->mBones[i]->mWeights[j].mWeight);

    WriteFloatArray( idstr + "-skin-weights", FloatType_Weight, (const ai_real*) skin_weights.data(), skin_weights.size());

    skin_weights.clear();

    mOutput << startstr << "<joints>" << endstr;
    PushTag();

    mOutput << startstr << "<input semantic=\"JOINT\" source=\"#" << idstrEscaped << "-skin-joints\"></input>" << endstr;
    mOutput << startstr << "<input semantic=\"INV_BIND_MATRIX\" source=\"#" << idstrEscaped << "-skin-bind_poses\"></input>" << endstr;

    PopTag();
    mOutput << startstr << "</joints>" << endstr;

    mOutput << startstr << "<vertex_weights count=\"" << mesh->mNumVertices << "\">" << endstr;
    PushTag();

    mOutput << startstr << "<input semantic=\"JOINT\" source=\"#" << idstrEscaped << "-skin-joints\" offset=\"0\"></input>" << endstr;
    mOutput << startstr << "<input semantic=\"WEIGHT\" source=\"#" << idstrEscaped << "-skin-weights\" offset=\"1\"></input>" << endstr;

    mOutput << startstr << "<vcount>";

    std::vector<ai_uint> num_influences(mesh->mNumVertices, (ai_uint)0);
    for( size_t i = 0; i < mesh->mNumBones; ++i)
        for( size_t j = 0; j < mesh->mBones[i]->mNumWeights; ++j)
            ++num_influences[mesh->mBones[i]->mWeights[j].mVertexId];

    for( size_t i = 0; i < mesh->mNumVertices; ++i)
        mOutput << num_influences[i] << " ";

    mOutput << "</vcount>" << endstr;

    mOutput << startstr << "<v>";

    ai_uint joint_weight_indices_length = 0;
    std::vector<ai_uint> accum_influences;
    accum_influences.reserve(num_influences.size());
    for( size_t i = 0; i < num_influences.size(); ++i)
    {
        accum_influences.push_back(joint_weight_indices_length);
        joint_weight_indices_length += num_influences[i];
    }

    ai_uint weight_index = 0;
    std::vector<ai_int> joint_weight_indices(2 * joint_weight_indices_length, (ai_int)-1);
    for( unsigned int i = 0; i < mesh->mNumBones; ++i)
        for( unsigned j = 0; j < mesh->mBones[i]->mNumWeights; ++j)
        {
            unsigned int vId = mesh->mBones[i]->mWeights[j].mVertexId;
            for( ai_uint k = 0; k < num_influences[vId]; ++k)
            {
                if (joint_weight_indices[2 * (accum_influences[vId] + k)] == -1)
                {
                    joint_weight_indices[2 * (accum_influences[vId] + k)] = i;
                    joint_weight_indices[2 * (accum_influences[vId] + k) + 1] = weight_index;
                    break;
                }
            }
            ++weight_index;
        }

    for( size_t i = 0; i < joint_weight_indices.size(); ++i)
        mOutput << joint_weight_indices[i] << " ";

    num_influences.clear();
    accum_influences.clear();
    joint_weight_indices.clear();

    mOutput << "</v>" << endstr;

    PopTag();
    mOutput << startstr << "</vertex_weights>" << endstr;

    PopTag();
    mOutput << startstr << "</skin>" << endstr;
    
    PopTag();
    mOutput << startstr << "</controller>" << endstr;
}

// ------------------------------------------------------------------------------------------------
// Writes the geometry library
void ColladaExporter::WriteGeometryLibrary()
{
    mOutput << startstr << "<library_geometries>" << endstr;
    PushTag();

    for( size_t a = 0; a < mScene->mNumMeshes; ++a)
        WriteGeometry( a);

    PopTag();
    mOutput << startstr << "</library_geometries>" << endstr;
}

// ------------------------------------------------------------------------------------------------
// Writes the given mesh
void ColladaExporter::WriteGeometry( size_t pIndex)
{
    const aiMesh* mesh = mScene->mMeshes[pIndex];
    const std::string idstr = GetMeshId( pIndex);
    const std::string idstrEscaped = XMLEscape(idstr);

    if ( mesh->mNumFaces == 0 || mesh->mNumVertices == 0 )
        return;

    // opening tag
    mOutput << startstr << "<geometry id=\"" << idstrEscaped << "\" name=\"" << idstrEscaped << "_name\" >" << endstr;
    PushTag();

    mOutput << startstr << "<mesh>" << endstr;
    PushTag();

    // Positions
    WriteFloatArray( idstr + "-positions", FloatType_Vector, (ai_real*) mesh->mVertices, mesh->mNumVertices);
    // Normals, if any
    if( mesh->HasNormals() )
        WriteFloatArray( idstr + "-normals", FloatType_Vector, (ai_real*) mesh->mNormals, mesh->mNumVertices);

    // texture coords
    for( size_t a = 0; a < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++a)
    {
        if( mesh->HasTextureCoords(static_cast<unsigned int>(a)) )
        {
            WriteFloatArray( idstr + "-tex" + to_string(a), mesh->mNumUVComponents[a] == 3 ? FloatType_TexCoord3 : FloatType_TexCoord2,
                (ai_real*) mesh->mTextureCoords[a], mesh->mNumVertices);
        }
    }

    // vertex colors
    for( size_t a = 0; a < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++a)
    {
        if( mesh->HasVertexColors(static_cast<unsigned int>(a)) )
            WriteFloatArray( idstr + "-color" + to_string(a), FloatType_Color, (ai_real*) mesh->mColors[a], mesh->mNumVertices);
    }

    // assemble vertex structure
    // Only write input for POSITION since we will write other as shared inputs in polygon definition
    mOutput << startstr << "<vertices id=\"" << idstrEscaped << "-vertices" << "\">" << endstr;
    PushTag();
    mOutput << startstr << "<input semantic=\"POSITION\" source=\"#" << idstrEscaped << "-positions\" />" << endstr;
    PopTag();
    mOutput << startstr << "</vertices>" << endstr;

    // count the number of lines, triangles and polygon meshes
    int countLines = 0;
    int countPoly = 0;
    for( size_t a = 0; a < mesh->mNumFaces; ++a )
    {
        if (mesh->mFaces[a].mNumIndices == 2) countLines++;
        else if (mesh->mFaces[a].mNumIndices >= 3) countPoly++;
    }

    // lines
    if (countLines)
    {
        mOutput << startstr << "<lines count=\"" << countLines << "\" material=\"defaultMaterial\">" << endstr;
        PushTag();
        mOutput << startstr << "<input offset=\"0\" semantic=\"VERTEX\" source=\"#" << idstrEscaped << "-vertices\" />" << endstr;
        if( mesh->HasNormals() )
            mOutput << startstr << "<input semantic=\"NORMAL\" source=\"#" << idstrEscaped << "-normals\" />" << endstr;
        for( size_t a = 0; a < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++a )
        {
            if( mesh->HasTextureCoords(static_cast<unsigned int>(a)) )
                mOutput << startstr << "<input semantic=\"TEXCOORD\" source=\"#" << idstrEscaped << "-tex" << a << "\" " << "set=\"" << a << "\""  << " />" << endstr;
        }
        for( size_t a = 0; a < AI_MAX_NUMBER_OF_COLOR_SETS; ++a )
        {
            if( mesh->HasVertexColors(static_cast<unsigned int>(a) ) )
                mOutput << startstr << "<input semantic=\"COLOR\" source=\"#" << idstrEscaped << "-color" << a << "\" " << "set=\"" << a << "\""  << " />" << endstr;
        }

        mOutput << startstr << "<p>";
        for( size_t a = 0; a < mesh->mNumFaces; ++a )
        {
            const aiFace& face = mesh->mFaces[a];
            if (face.mNumIndices != 2) continue;
            for( size_t b = 0; b < face.mNumIndices; ++b )
                mOutput << face.mIndices[b] << " ";
        }
        mOutput << "</p>" << endstr;
        PopTag();
        mOutput << startstr << "</lines>" << endstr;
    }

    // triangle - don't use it, because compatibility problems

    // polygons
    if (countPoly)
    {
        mOutput << startstr << "<polylist count=\"" << countPoly << "\" material=\"defaultMaterial\">" << endstr;
        PushTag();
        mOutput << startstr << "<input offset=\"0\" semantic=\"VERTEX\" source=\"#" << idstrEscaped << "-vertices\" />" << endstr;
        if( mesh->HasNormals() )
            mOutput << startstr << "<input offset=\"0\" semantic=\"NORMAL\" source=\"#" << idstrEscaped << "-normals\" />" << endstr;
        for( size_t a = 0; a < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++a )
        {
            if( mesh->HasTextureCoords(static_cast<unsigned int>(a)) )
                mOutput << startstr << "<input offset=\"0\" semantic=\"TEXCOORD\" source=\"#" << idstrEscaped << "-tex" << a << "\" " << "set=\"" << a << "\""  << " />" << endstr;
        }
        for( size_t a = 0; a < AI_MAX_NUMBER_OF_COLOR_SETS; ++a )
        {
            if( mesh->HasVertexColors(static_cast<unsigned int>(a) ) )
                mOutput << startstr << "<input offset=\"0\" semantic=\"COLOR\" source=\"#" << idstrEscaped << "-color" << a << "\" " << "set=\"" << a << "\""  << " />" << endstr;
        }

        mOutput << startstr << "<vcount>";
        for( size_t a = 0; a < mesh->mNumFaces; ++a )
        {
            if (mesh->mFaces[a].mNumIndices < 3) continue;
            mOutput << mesh->mFaces[a].mNumIndices << " ";
        }
        mOutput << "</vcount>" << endstr;

        mOutput << startstr << "<p>";
        for( size_t a = 0; a < mesh->mNumFaces; ++a )
        {
            const aiFace& face = mesh->mFaces[a];
            if (face.mNumIndices < 3) continue;
            for( size_t b = 0; b < face.mNumIndices; ++b )
                mOutput << face.mIndices[b] << " ";
        }
        mOutput << "</p>" << endstr;
        PopTag();
        mOutput << startstr << "</polylist>" << endstr;
    }

    // closing tags
    PopTag();
    mOutput << startstr << "</mesh>" << endstr;
    PopTag();
    mOutput << startstr << "</geometry>" << endstr;
}

// ------------------------------------------------------------------------------------------------
// Writes a float array of the given type
void ColladaExporter::WriteFloatArray( const std::string& pIdString, FloatDataType pType, const ai_real* pData, size_t pElementCount)
{
    size_t floatsPerElement = 0;
    switch( pType )
    {
        case FloatType_Vector: floatsPerElement = 3; break;
        case FloatType_TexCoord2: floatsPerElement = 2; break;
        case FloatType_TexCoord3: floatsPerElement = 3; break;
        case FloatType_Color: floatsPerElement = 3; break;
        case FloatType_Mat4x4: floatsPerElement = 16; break;
        case FloatType_Weight: floatsPerElement = 1; break;
		case FloatType_Time: floatsPerElement = 1; break;
        default:
            return;
    }

    std::string arrayId = pIdString + "-array";

    mOutput << startstr << "<source id=\"" << XMLEscape(pIdString) << "\" name=\"" << XMLEscape(pIdString) << "\">" << endstr;
    PushTag();

    // source array
    mOutput << startstr << "<float_array id=\"" << XMLEscape(arrayId) << "\" count=\"" << pElementCount * floatsPerElement << "\"> ";
    PushTag();

    if( pType == FloatType_TexCoord2 )
    {
        for( size_t a = 0; a < pElementCount; ++a )
        {
            mOutput << pData[a*3+0] << " ";
            mOutput << pData[a*3+1] << " ";
        }
    }
    else if( pType == FloatType_Color )
    {
        for( size_t a = 0; a < pElementCount; ++a )
        {
            mOutput << pData[a*4+0] << " ";
            mOutput << pData[a*4+1] << " ";
            mOutput << pData[a*4+2] << " ";
        }
    }
    else
    {
        for( size_t a = 0; a < pElementCount * floatsPerElement; ++a )
            mOutput << pData[a] << " ";
    }
    mOutput << "</float_array>" << endstr;
    PopTag();

    // the usual Collada fun. Let's bloat it even more!
    mOutput << startstr << "<technique_common>" << endstr;
    PushTag();
    mOutput << startstr << "<accessor count=\"" << pElementCount << "\" offset=\"0\" source=\"#" << arrayId << "\" stride=\"" << floatsPerElement << "\">" << endstr;
    PushTag();

    switch( pType )
    {
        case FloatType_Vector:
            mOutput << startstr << "<param name=\"X\" type=\"float\" />" << endstr;
            mOutput << startstr << "<param name=\"Y\" type=\"float\" />" << endstr;
            mOutput << startstr << "<param name=\"Z\" type=\"float\" />" << endstr;
            break;

        case FloatType_TexCoord2:
            mOutput << startstr << "<param name=\"S\" type=\"float\" />" << endstr;
            mOutput << startstr << "<param name=\"T\" type=\"float\" />" << endstr;
            break;

        case FloatType_TexCoord3:
            mOutput << startstr << "<param name=\"S\" type=\"float\" />" << endstr;
            mOutput << startstr << "<param name=\"T\" type=\"float\" />" << endstr;
            mOutput << startstr << "<param name=\"P\" type=\"float\" />" << endstr;
            break;

        case FloatType_Color:
            mOutput << startstr << "<param name=\"R\" type=\"float\" />" << endstr;
            mOutput << startstr << "<param name=\"G\" type=\"float\" />" << endstr;
            mOutput << startstr << "<param name=\"B\" type=\"float\" />" << endstr;
            break;

        case FloatType_Mat4x4:
            mOutput << startstr << "<param name=\"TRANSFORM\" type=\"float4x4\" />" << endstr;
            break;

        case FloatType_Weight:
            mOutput << startstr << "<param name=\"WEIGHT\" type=\"float\" />" << endstr;
            break;

		// customized, add animation related
		case FloatType_Time:
			mOutput << startstr << "<param name=\"TIME\" type=\"float\" />" << endstr;
			break;

	}

    PopTag();
    mOutput << startstr << "</accessor>" << endstr;
    PopTag();
    mOutput << startstr << "</technique_common>" << endstr;
    PopTag();
    mOutput << startstr << "</source>" << endstr;
}

// ------------------------------------------------------------------------------------------------
// Writes the scene library
void ColladaExporter::WriteSceneLibrary()
{
    const std::string scene_name_escaped = XMLEscape(mScene->mRootNode->mName.C_Str());

    mOutput << startstr << "<library_visual_scenes>" << endstr;
    PushTag();
    mOutput << startstr << "<visual_scene id=\"" + scene_name_escaped + "\" name=\"" + scene_name_escaped + "\">" << endstr;
    PushTag();

    // start recursive write at the root node
    for( size_t a = 0; a < mScene->mRootNode->mNumChildren; ++a )
        WriteNode( mScene, mScene->mRootNode->mChildren[a]);

    PopTag();
    mOutput << startstr << "</visual_scene>" << endstr;
    PopTag();
    mOutput << startstr << "</library_visual_scenes>" << endstr;
}
// ------------------------------------------------------------------------------------------------
void ColladaExporter::WriteAnimationLibrary(size_t pIndex)
{
	const aiAnimation * anim = mScene->mAnimations[pIndex];
	
	if ( anim->mNumChannels == 0 && anim->mNumMeshChannels == 0 && anim->mNumMorphMeshChannels ==0 )
		return;
	
	const std::string animation_name_escaped = XMLEscape( anim->mName.C_Str() );
	std::string idstr = anim->mName.C_Str();
	std::string ending = std::string( "AnimId" ) + to_string(pIndex);
	if (idstr.length() >= ending.length()) {
		if (0 != idstr.compare (idstr.length() - ending.length(), ending.length(), ending)) {
			idstr = idstr + ending;
		}
	} else {
		idstr = idstr + ending;
	}

	const std::string idstrEscaped = XMLEscape(idstr);
	
	mOutput << startstr << "<animation id=\"" + idstrEscaped + "\" name=\"" + animation_name_escaped + "\">" << endstr;
	PushTag();

    std::string node_idstr;
	for (size_t a = 0; a < anim->mNumChannels; ++a) {
		const aiNodeAnim * nodeAnim = anim->mChannels[a];
		
		// sanity check
		if ( nodeAnim->mNumPositionKeys != nodeAnim->mNumScalingKeys ||  nodeAnim->mNumPositionKeys != nodeAnim->mNumRotationKeys ) continue;
		
		{
            node_idstr.clear();
            node_idstr += nodeAnim->mNodeName.data;
            node_idstr += std::string( "_matrix-input" );

			std::vector<ai_real> frames;
			for( size_t i = 0; i < nodeAnim->mNumPositionKeys; ++i) {
				frames.push_back(static_cast<ai_real>(nodeAnim->mPositionKeys[i].mTime));
			}
			
			WriteFloatArray( node_idstr , FloatType_Time, (const ai_real*) frames.data(), frames.size());
			frames.clear();
		}
		
		{
            node_idstr.clear();

            node_idstr += nodeAnim->mNodeName.data;
            node_idstr += std::string("_matrix-output");
			
			std::vector<ai_real> keyframes;
			keyframes.reserve(nodeAnim->mNumPositionKeys * 16);
			for( size_t i = 0; i < nodeAnim->mNumPositionKeys; ++i) {
				aiVector3D Scaling = nodeAnim->mScalingKeys[i].mValue;
				aiMatrix4x4 ScalingM;  // identity
				ScalingM[0][0] = Scaling.x; ScalingM[1][1] = Scaling.y; ScalingM[2][2] = Scaling.z;
				
				aiQuaternion RotationQ = nodeAnim->mRotationKeys[i].mValue;
				aiMatrix4x4 s = aiMatrix4x4( RotationQ.GetMatrix() );
				aiMatrix4x4 RotationM(s.a1, s.a2, s.a3, 0, s.b1, s.b2, s.b3, 0, s.c1, s.c2, s.c3, 0, 0, 0, 0, 1);
				
				aiVector3D Translation = nodeAnim->mPositionKeys[i].mValue;
				aiMatrix4x4 TranslationM;	// identity
				TranslationM[0][3] = Translation.x; TranslationM[1][3] = Translation.y; TranslationM[2][3] = Translation.z;
				
				// Combine the above transformations
				aiMatrix4x4 mat = TranslationM * RotationM * ScalingM;
				
				for( unsigned int j = 0; j < 4; ++j) {
					keyframes.insert(keyframes.end(), mat[j], mat[j] + 4);
                }
			}
			
			WriteFloatArray( node_idstr, FloatType_Mat4x4, (const ai_real*) keyframes.data(), keyframes.size() / 16);
		}
		
		{
			std::vector<std::string> names;
			for ( size_t i = 0; i < nodeAnim->mNumPositionKeys; ++i) {
				if ( nodeAnim->mPreState == aiAnimBehaviour_DEFAULT
					|| nodeAnim->mPreState == aiAnimBehaviour_LINEAR
					|| nodeAnim->mPreState == aiAnimBehaviour_REPEAT
					) {
					names.push_back( "LINEAR" );
				} else if (nodeAnim->mPostState == aiAnimBehaviour_CONSTANT) {
					names.push_back( "STEP" );
				}
			}
			
			const std::string node_idstr = nodeAnim->mNodeName.data + std::string("_matrix-interpolation");
			std::string arrayId = node_idstr + "-array";
			
			mOutput << startstr << "<source id=\"" << XMLEscape(node_idstr) << "\">" << endstr;
			PushTag();
			
			// source array
			mOutput << startstr << "<Name_array id=\"" << XMLEscape(arrayId) << "\" count=\"" << names.size() << "\"> ";
			for( size_t a = 0; a < names.size(); ++a ) {
				mOutput << names[a] << " ";
            }
			mOutput << "</Name_array>" << endstr;
			
			mOutput << startstr << "<technique_common>" << endstr;
			PushTag();

			mOutput << startstr << "<accessor source=\"#" << XMLEscape(arrayId) << "\" count=\"" << names.size() << "\" stride=\"" << 1 << "\">" << endstr;
			PushTag();
			
			mOutput << startstr << "<param name=\"INTERPOLATION\" type=\"name\"></param>" << endstr;
			
			PopTag();
			mOutput << startstr << "</accessor>" << endstr;
			
			PopTag();
			mOutput << startstr << "</technique_common>" << endstr;

			PopTag();
			mOutput << startstr << "</source>" << endstr;
		}
	}
	
	for (size_t a = 0; a < anim->mNumChannels; ++a) {
		const aiNodeAnim * nodeAnim = anim->mChannels[a];
		
		{
		// samplers
			const std::string node_idstr = nodeAnim->mNodeName.data + std::string("_matrix-sampler");
			mOutput << startstr << "<sampler id=\"" << XMLEscape(node_idstr) << "\">" << endstr;
			PushTag();
			
			mOutput << startstr << "<input semantic=\"INPUT\" source=\"#" << XMLEscape( nodeAnim->mNodeName.data + std::string("_matrix-input") ) << "\"/>" << endstr;
			mOutput << startstr << "<input semantic=\"OUTPUT\" source=\"#" << XMLEscape( nodeAnim->mNodeName.data + std::string("_matrix-output") ) << "\"/>" << endstr;
			mOutput << startstr << "<input semantic=\"INTERPOLATION\" source=\"#" << XMLEscape( nodeAnim->mNodeName.data + std::string("_matrix-interpolation") ) << "\"/>" << endstr;
			
			PopTag();
			mOutput << startstr << "</sampler>" << endstr;
		}
	}
	
	for (size_t a = 0; a < anim->mNumChannels; ++a) {
		const aiNodeAnim * nodeAnim = anim->mChannels[a];
		
		{
		// channels
			mOutput << startstr << "<channel source=\"#" << XMLEscape( nodeAnim->mNodeName.data + std::string("_matrix-sampler") ) << "\" target=\"" << XMLEscape(nodeAnim->mNodeName.data) << "/matrix\"/>" << endstr;
		}
	}
	
	PopTag();
	mOutput << startstr << "</animation>" << endstr;
	
}
// ------------------------------------------------------------------------------------------------
void ColladaExporter::WriteAnimationsLibrary()
{
	const std::string scene_name_escaped = XMLEscape(mScene->mRootNode->mName.C_Str());
	
	if ( mScene->mNumAnimations > 0 ) {
		mOutput << startstr << "<library_animations>" << endstr;
		PushTag();
		
		// start recursive write at the root node
		for( size_t a = 0; a < mScene->mNumAnimations; ++a)
			WriteAnimationLibrary( a );

		PopTag();
		mOutput << startstr << "</library_animations>" << endstr;
	}
}
// ------------------------------------------------------------------------------------------------
// Helper to find a bone by name in the scene
aiBone* findBone( const aiScene* scene, const char * name) {
    for (size_t m=0; m<scene->mNumMeshes; m++) {
        aiMesh * mesh = scene->mMeshes[m];
        for (size_t b=0; b<mesh->mNumBones; b++) {
            aiBone * bone = mesh->mBones[b];
            if (0 == strcmp(name, bone->mName.C_Str())) {
                return bone;
            }
        }
    }
    return NULL;
}

// ------------------------------------------------------------------------------------------------
const aiNode * findBoneNode( const aiNode* aNode, const aiBone* bone)
{
	if ( aNode && bone && aNode->mName == bone->mName ) {
		return aNode;
	}
	
	if ( aNode && bone ) {
		for (unsigned int i=0; i < aNode->mNumChildren; ++i) {
			aiNode * aChild = aNode->mChildren[i];
			const aiNode * foundFromChild = 0;
			if ( aChild ) {
				foundFromChild = findBoneNode( aChild, bone );
				if ( foundFromChild ) return foundFromChild;
			}
		}
	}
	
	return NULL;
}

const aiNode * findSkeletonRootNode( const aiScene* scene, const aiMesh * mesh)
{
	std::set<const aiNode*> topParentBoneNodes;
	if ( mesh && mesh->mNumBones > 0 ) {
		for (unsigned int i=0; i < mesh->mNumBones; ++i) {
			aiBone * bone = mesh->mBones[i];

			const aiNode * node = findBoneNode( scene->mRootNode, bone);
			if ( node ) {
				while ( node->mParent && findBone(scene, node->mParent->mName.C_Str() ) != 0 ) {
					node = node->mParent;
				}
				topParentBoneNodes.insert( node );
			}
		}
	}
	
	if ( !topParentBoneNodes.empty() ) {
		const aiNode * parentBoneNode = *topParentBoneNodes.begin();
		if ( topParentBoneNodes.size() == 1 ) {
			return parentBoneNode;
		} else {
			for (auto it : topParentBoneNodes) {
				if ( it->mParent ) return it->mParent;
			}
			return parentBoneNode;
		}
	}
	
	return NULL;
}

// ------------------------------------------------------------------------------------------------
// Recursively writes the given node
void ColladaExporter::WriteNode( const aiScene* pScene, aiNode* pNode)
{
    // the node must have a name
    if (pNode->mName.length == 0)
    {
        std::stringstream ss;
        ss << "Node_" << pNode;
        pNode->mName.Set(ss.str());
    }

    // If the node is associated with a bone, it is a joint node (JOINT)
    // otherwise it is a normal node (NODE)
    const char * node_type;
    bool is_joint, is_skeleton_root = false;
    if (NULL == findBone(pScene, pNode->mName.C_Str())) {
        node_type = "NODE";
        is_joint = false;
    } else {
        node_type = "JOINT";
        is_joint = true;
        if(!pNode->mParent || NULL == findBone(pScene, pNode->mParent->mName.C_Str()))
            is_skeleton_root = true;
    }

    const std::string node_name_escaped = XMLEscape(pNode->mName.data);
	/* // customized, Note! the id field is crucial for inter-xml look up, it cannot be replaced with sid ?!
    mOutput << startstr
            << "<node ";
    if(is_skeleton_root)
        mOutput << "id=\"" << "skeleton_root" << "\" "; // For now, only support one skeleton in a scene.
    mOutput << (is_joint ? "s" : "") << "id=\"" << node_name_escaped;
	 */
	mOutput << startstr << "<node ";
	if(is_skeleton_root) {
		mOutput << "id=\"" << node_name_escaped << "\" " << (is_joint ? "sid=\"" + node_name_escaped +"\"" : "") ; // For now, only support one skeleton in a scene.
		mFoundSkeletonRootNodeID = node_name_escaped;
	} else {
		mOutput << "id=\"" << node_name_escaped << "\" " << (is_joint ? "sid=\"" + node_name_escaped +"\"": "") ;
	}
	
    mOutput << " name=\"" << node_name_escaped
            << "\" type=\"" << node_type
            << "\">" << endstr;
    PushTag();

    // write transformation - we can directly put the matrix there
    // TODO: (thom) decompose into scale - rot - quad to allow addressing it by animations afterwards
    const aiMatrix4x4& mat = pNode->mTransformation;
	
	// customized, sid should be 'matrix' to match with loader code.
    //mOutput << startstr << "<matrix sid=\"transform\">";
	mOutput << startstr << "<matrix sid=\"matrix\">";
	
    mOutput << mat.a1 << " " << mat.a2 << " " << mat.a3 << " " << mat.a4 << " ";
    mOutput << mat.b1 << " " << mat.b2 << " " << mat.b3 << " " << mat.b4 << " ";
    mOutput << mat.c1 << " " << mat.c2 << " " << mat.c3 << " " << mat.c4 << " ";
    mOutput << mat.d1 << " " << mat.d2 << " " << mat.d3 << " " << mat.d4;
    mOutput << "</matrix>" << endstr;

    if(pNode->mNumMeshes==0){
        //check if it is a camera node
        for(size_t i=0; i<mScene->mNumCameras; i++){
            if(mScene->mCameras[i]->mName == pNode->mName){
                mOutput << startstr <<"<instance_camera url=\"#" << node_name_escaped << "-camera\"/>" << endstr;
                break;
            }
        }
        //check if it is a light node
        for(size_t i=0; i<mScene->mNumLights; i++){
            if(mScene->mLights[i]->mName == pNode->mName){
                mOutput << startstr <<"<instance_light url=\"#" << node_name_escaped << "-light\"/>" << endstr;
                break;
            }
        }

    }else
    // instance every geometry
    for( size_t a = 0; a < pNode->mNumMeshes; ++a )
    {
        const aiMesh* mesh = mScene->mMeshes[pNode->mMeshes[a]];
        // do not instantiate mesh if empty. I wonder how this could happen
        if( mesh->mNumFaces == 0 || mesh->mNumVertices == 0 )
            continue;

        if( mesh->mNumBones == 0 )
        {
            mOutput << startstr << "<instance_geometry url=\"#" << XMLEscape(GetMeshId( pNode->mMeshes[a])) << "\">" << endstr;
            PushTag();
        }
        else
        {
            mOutput << startstr
                    << "<instance_controller url=\"#" << XMLEscape(GetMeshId( pNode->mMeshes[a])) << "-skin\">"
                    << endstr;
            PushTag();

			// note! this mFoundSkeletonRootNodeID some how affects animation, it makes the mesh attaches to armature skeleton root node.
			// use the first bone to find skeleton root
			const aiNode * skeletonRootBoneNode = findSkeletonRootNode( pScene, mesh );
			if ( skeletonRootBoneNode ) {
				mFoundSkeletonRootNodeID = XMLEscape( skeletonRootBoneNode->mName.C_Str() );
			}
            mOutput << startstr << "<skeleton>#" << mFoundSkeletonRootNodeID << "</skeleton>" << endstr;
        }
        mOutput << startstr << "<bind_material>" << endstr;
        PushTag();
        mOutput << startstr << "<technique_common>" << endstr;
        PushTag();
        mOutput << startstr << "<instance_material symbol=\"defaultMaterial\" target=\"#" << XMLEscape(materials[mesh->mMaterialIndex].name) << "\">" << endstr;
        PushTag();
        for( size_t a = 0; a < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++a )
        {
            if( mesh->HasTextureCoords( static_cast<unsigned int>(a) ) )
                // semantic       as in <texture texcoord=...>
                // input_semantic as in <input semantic=...>
                // input_set      as in <input set=...>
                mOutput << startstr << "<bind_vertex_input semantic=\"CHANNEL" << a << "\" input_semantic=\"TEXCOORD\" input_set=\"" << a << "\"/>" << endstr;
        }
        PopTag();
        mOutput << startstr << "</instance_material>" << endstr;
        PopTag();
        mOutput << startstr << "</technique_common>" << endstr;
        PopTag();
        mOutput << startstr << "</bind_material>" << endstr;
        
        PopTag();
        if( mesh->mNumBones == 0)
            mOutput << startstr << "</instance_geometry>" << endstr;
        else
            mOutput << startstr << "</instance_controller>" << endstr;
    }

    // recurse into subnodes
    for( size_t a = 0; a < pNode->mNumChildren; ++a )
        WriteNode( pScene, pNode->mChildren[a]);

    PopTag();
    mOutput << startstr << "</node>" << endstr;
}

#endif
#endif
