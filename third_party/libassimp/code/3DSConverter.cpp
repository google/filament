/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team


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

/** @file Implementation of the 3ds importer class */


#ifndef ASSIMP_BUILD_NO_3DS_IMPORTER

// internal headers
#include "3DSLoader.h"
#include "TargetAnimation.h"
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include "StringComparison.h"
#include <memory>
#include <cctype>

using namespace Assimp;

static const unsigned int NotSet = 0xcdcdcdcd;

// ------------------------------------------------------------------------------------------------
// Setup final material indices, generae a default material if necessary
void Discreet3DSImporter::ReplaceDefaultMaterial()
{
    // Try to find an existing material that matches the
    // typical default material setting:
    // - no textures
    // - diffuse color (in grey!)
    // NOTE: This is here to workaround the fact that some
    // exporters are writing a default material, too.
    unsigned int idx( NotSet );
    for (unsigned int i = 0; i < mScene->mMaterials.size();++i)
    {
        std::string s = mScene->mMaterials[i].mName;
        for ( std::string::iterator it = s.begin(); it != s.end(); ++it ) {
            *it = static_cast< char >( ::tolower( *it ) );
        }

        if (std::string::npos == s.find("default"))continue;

        if (mScene->mMaterials[i].mDiffuse.r !=
            mScene->mMaterials[i].mDiffuse.g ||
            mScene->mMaterials[i].mDiffuse.r !=
            mScene->mMaterials[i].mDiffuse.b)continue;

        if (mScene->mMaterials[i].sTexDiffuse.mMapName.length()   != 0  ||
            mScene->mMaterials[i].sTexBump.mMapName.length()      != 0  ||
            mScene->mMaterials[i].sTexOpacity.mMapName.length()   != 0  ||
            mScene->mMaterials[i].sTexEmissive.mMapName.length()  != 0  ||
            mScene->mMaterials[i].sTexSpecular.mMapName.length()  != 0  ||
            mScene->mMaterials[i].sTexShininess.mMapName.length() != 0 )
        {
            continue;
        }
        idx = i;
    }
    if ( NotSet == idx ) {
        idx = ( unsigned int )mScene->mMaterials.size();
    }

    // now iterate through all meshes and through all faces and
    // find all faces that are using the default material
    unsigned int cnt = 0;
    for (std::vector<D3DS::Mesh>::iterator
        i =  mScene->mMeshes.begin();
        i != mScene->mMeshes.end();++i)
    {
        for (std::vector<unsigned int>::iterator
            a =  (*i).mFaceMaterials.begin();
            a != (*i).mFaceMaterials.end();++a)
        {
            // NOTE: The additional check seems to be necessary,
            // some exporters seem to generate invalid data here
            if (0xcdcdcdcd == (*a))
            {
                (*a) = idx;
                ++cnt;
            }
            else if ( (*a) >= mScene->mMaterials.size())
            {
                (*a) = idx;
                DefaultLogger::get()->warn("Material index overflow in 3DS file. Using default material");
                ++cnt;
            }
        }
    }
    if (cnt && idx == mScene->mMaterials.size())
    {
        // We need to create our own default material
        D3DS::Material sMat;
        sMat.mDiffuse = aiColor3D(0.3f,0.3f,0.3f);
        sMat.mName = "%%%DEFAULT";
        mScene->mMaterials.push_back(sMat);

        DefaultLogger::get()->info("3DS: Generating default material");
    }
}

// ------------------------------------------------------------------------------------------------
// Check whether all indices are valid. Otherwise we'd crash before the validation step is reached
void Discreet3DSImporter::CheckIndices(D3DS::Mesh& sMesh)
{
    for (std::vector< D3DS::Face >::iterator i =  sMesh.mFaces.begin(); i != sMesh.mFaces.end();++i)
    {
        // check whether all indices are in range
        for (unsigned int a = 0; a < 3;++a)
        {
            if ((*i).mIndices[a] >= sMesh.mPositions.size())
            {
                DefaultLogger::get()->warn("3DS: Vertex index overflow)");
                (*i).mIndices[a] = (uint32_t)sMesh.mPositions.size()-1;
            }
            if ( !sMesh.mTexCoords.empty() && (*i).mIndices[a] >= sMesh.mTexCoords.size())
            {
                DefaultLogger::get()->warn("3DS: Texture coordinate index overflow)");
                (*i).mIndices[a] = (uint32_t)sMesh.mTexCoords.size()-1;
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Generate out unique verbose format representation
void Discreet3DSImporter::MakeUnique(D3DS::Mesh& sMesh)
{
    // TODO: really necessary? I don't think. Just a waste of memory and time
    // to do it now in a separate buffer.

    // Allocate output storage
    std::vector<aiVector3D> vNew  (sMesh.mFaces.size() * 3);
    std::vector<aiVector3D> vNew2;
    if (sMesh.mTexCoords.size())
        vNew2.resize(sMesh.mFaces.size() * 3);

    for (unsigned int i = 0, base = 0; i < sMesh.mFaces.size();++i)
    {
        D3DS::Face& face = sMesh.mFaces[i];

        // Positions
        for (unsigned int a = 0; a < 3;++a,++base)
        {
            vNew[base] = sMesh.mPositions[face.mIndices[a]];
            if (sMesh.mTexCoords.size())
                vNew2[base] = sMesh.mTexCoords[face.mIndices[a]];

            face.mIndices[a] = base;
        }
    }
    sMesh.mPositions = vNew;
    sMesh.mTexCoords = vNew2;
}

// ------------------------------------------------------------------------------------------------
// Convert a 3DS texture to texture keys in an aiMaterial
void CopyTexture(aiMaterial& mat, D3DS::Texture& texture, aiTextureType type)
{
    // Setup the texture name
    aiString tex;
    tex.Set( texture.mMapName);
    mat.AddProperty( &tex, AI_MATKEY_TEXTURE(type,0));

    // Setup the texture blend factor
    if (is_not_qnan(texture.mTextureBlend))
        mat.AddProperty<ai_real>( &texture.mTextureBlend, 1, AI_MATKEY_TEXBLEND(type,0));

    // Setup the texture mapping mode
    mat.AddProperty<int>((int*)&texture.mMapMode,1,AI_MATKEY_MAPPINGMODE_U(type,0));
    mat.AddProperty<int>((int*)&texture.mMapMode,1,AI_MATKEY_MAPPINGMODE_V(type,0));

    // Mirroring - double the scaling values
    // FIXME: this is not really correct ...
    if (texture.mMapMode == aiTextureMapMode_Mirror)
    {
        texture.mScaleU *= 2.0;
        texture.mScaleV *= 2.0;
        texture.mOffsetU /= 2.0;
        texture.mOffsetV /= 2.0;
    }

    // Setup texture UV transformations
    mat.AddProperty<ai_real>(&texture.mOffsetU,5,AI_MATKEY_UVTRANSFORM(type,0));
}

// ------------------------------------------------------------------------------------------------
// Convert a 3DS material to an aiMaterial
void Discreet3DSImporter::ConvertMaterial(D3DS::Material& oldMat,
    aiMaterial& mat)
{
    // NOTE: Pass the background image to the viewer by bypassing the
    // material system. This is an evil hack, never do it again!
    if (0 != mBackgroundImage.length() && bHasBG)
    {
        aiString tex;
        tex.Set( mBackgroundImage);
        mat.AddProperty( &tex, AI_MATKEY_GLOBAL_BACKGROUND_IMAGE);

        // Be sure this is only done for the first material
        mBackgroundImage = std::string("");
    }

    // At first add the base ambient color of the scene to the material
    oldMat.mAmbient.r += mClrAmbient.r;
    oldMat.mAmbient.g += mClrAmbient.g;
    oldMat.mAmbient.b += mClrAmbient.b;

    aiString name;
    name.Set( oldMat.mName);
    mat.AddProperty( &name, AI_MATKEY_NAME);

    // Material colors
    mat.AddProperty( &oldMat.mAmbient, 1, AI_MATKEY_COLOR_AMBIENT);
    mat.AddProperty( &oldMat.mDiffuse, 1, AI_MATKEY_COLOR_DIFFUSE);
    mat.AddProperty( &oldMat.mSpecular, 1, AI_MATKEY_COLOR_SPECULAR);
    mat.AddProperty( &oldMat.mEmissive, 1, AI_MATKEY_COLOR_EMISSIVE);

    // Phong shininess and shininess strength
    if (D3DS::Discreet3DS::Phong == oldMat.mShading ||
        D3DS::Discreet3DS::Metal == oldMat.mShading)
    {
        if (!oldMat.mSpecularExponent || !oldMat.mShininessStrength)
        {
            oldMat.mShading = D3DS::Discreet3DS::Gouraud;
        }
        else
        {
            mat.AddProperty( &oldMat.mSpecularExponent, 1, AI_MATKEY_SHININESS);
            mat.AddProperty( &oldMat.mShininessStrength, 1, AI_MATKEY_SHININESS_STRENGTH);
        }
    }

    // Opacity
    mat.AddProperty<ai_real>( &oldMat.mTransparency,1,AI_MATKEY_OPACITY);

    // Bump height scaling
    mat.AddProperty<ai_real>( &oldMat.mBumpHeight,1,AI_MATKEY_BUMPSCALING);

    // Two sided rendering?
    if (oldMat.mTwoSided)
    {
        int i = 1;
        mat.AddProperty<int>(&i,1,AI_MATKEY_TWOSIDED);
    }

    // Shading mode
    aiShadingMode eShading = aiShadingMode_NoShading;
    switch (oldMat.mShading)
    {
        case D3DS::Discreet3DS::Flat:
            eShading = aiShadingMode_Flat; break;

        // I don't know what "Wire" shading should be,
        // assume it is simple lambertian diffuse shading
        case D3DS::Discreet3DS::Wire:
            {
                // Set the wireframe flag
                unsigned int iWire = 1;
                mat.AddProperty<int>( (int*)&iWire,1,AI_MATKEY_ENABLE_WIREFRAME);
            }

        case D3DS::Discreet3DS::Gouraud:
            eShading = aiShadingMode_Gouraud; break;

        // assume cook-torrance shading for metals.
        case D3DS::Discreet3DS::Phong :
            eShading = aiShadingMode_Phong; break;

        case D3DS::Discreet3DS::Metal :
            eShading = aiShadingMode_CookTorrance; break;

            // FIX to workaround a warning with GCC 4 who complained
            // about a missing case Blinn: here - Blinn isn't a valid
            // value in the 3DS Loader, it is just needed for ASE
        case D3DS::Discreet3DS::Blinn :
            eShading = aiShadingMode_Blinn; break;
    }
    mat.AddProperty<int>( (int*)&eShading,1,AI_MATKEY_SHADING_MODEL);

    // DIFFUSE texture
    if( oldMat.sTexDiffuse.mMapName.length() > 0)
        CopyTexture(mat,oldMat.sTexDiffuse, aiTextureType_DIFFUSE);

    // SPECULAR texture
    if( oldMat.sTexSpecular.mMapName.length() > 0)
        CopyTexture(mat,oldMat.sTexSpecular, aiTextureType_SPECULAR);

    // OPACITY texture
    if( oldMat.sTexOpacity.mMapName.length() > 0)
        CopyTexture(mat,oldMat.sTexOpacity, aiTextureType_OPACITY);

    // EMISSIVE texture
    if( oldMat.sTexEmissive.mMapName.length() > 0)
        CopyTexture(mat,oldMat.sTexEmissive, aiTextureType_EMISSIVE);

    // BUMP texture
    if( oldMat.sTexBump.mMapName.length() > 0)
        CopyTexture(mat,oldMat.sTexBump, aiTextureType_HEIGHT);

    // SHININESS texture
    if( oldMat.sTexShininess.mMapName.length() > 0)
        CopyTexture(mat,oldMat.sTexShininess, aiTextureType_SHININESS);

    // REFLECTION texture
    if( oldMat.sTexReflective.mMapName.length() > 0)
        CopyTexture(mat,oldMat.sTexReflective, aiTextureType_REFLECTION);

    // Store the name of the material itself, too
    if( oldMat.mName.length())  {
        aiString tex;
        tex.Set( oldMat.mName);
        mat.AddProperty( &tex, AI_MATKEY_NAME);
    }
}

// ------------------------------------------------------------------------------------------------
// Split meshes by their materials and generate output aiMesh'es
void Discreet3DSImporter::ConvertMeshes(aiScene* pcOut)
{
    std::vector<aiMesh*> avOutMeshes;
    avOutMeshes.reserve(mScene->mMeshes.size() * 2);

    unsigned int iFaceCnt = 0,num = 0;
    aiString name;

    // we need to split all meshes by their materials
    for (std::vector<D3DS::Mesh>::iterator i =  mScene->mMeshes.begin(); i != mScene->mMeshes.end();++i)    {
        std::unique_ptr< std::vector<unsigned int>[] > aiSplit(new std::vector<unsigned int>[mScene->mMaterials.size()]);

        name.length = ASSIMP_itoa10(name.data,num++);

        unsigned int iNum = 0;
        for (std::vector<unsigned int>::const_iterator a =  (*i).mFaceMaterials.begin();
            a != (*i).mFaceMaterials.end();++a,++iNum)
        {
            aiSplit[*a].push_back(iNum);
        }
        // now generate submeshes
        for (unsigned int p = 0; p < mScene->mMaterials.size();++p)
        {
            if (aiSplit[p].empty()) {
                continue;
            }
            aiMesh* meshOut = new aiMesh();
            meshOut->mName = name;
            meshOut->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

            // be sure to setup the correct material index
            meshOut->mMaterialIndex = p;

            // use the color data as temporary storage
            meshOut->mColors[0] = (aiColor4D*)(&*i);
            avOutMeshes.push_back(meshOut);

            // convert vertices
            meshOut->mNumFaces = (unsigned int)aiSplit[p].size();
            meshOut->mNumVertices = meshOut->mNumFaces*3;

            // allocate enough storage for faces
            meshOut->mFaces = new aiFace[meshOut->mNumFaces];
            iFaceCnt += meshOut->mNumFaces;

            meshOut->mVertices = new aiVector3D[meshOut->mNumVertices];
            meshOut->mNormals  = new aiVector3D[meshOut->mNumVertices];
            if ((*i).mTexCoords.size())
            {
                meshOut->mTextureCoords[0] = new aiVector3D[meshOut->mNumVertices];
            }
            for (unsigned int q = 0, base = 0; q < aiSplit[p].size();++q)
            {
                unsigned int index = aiSplit[p][q];
                aiFace& face = meshOut->mFaces[q];

                face.mIndices = new unsigned int[3];
                face.mNumIndices = 3;

                for (unsigned int a = 0; a < 3;++a,++base)
                {
                    unsigned int idx = (*i).mFaces[index].mIndices[a];
                    meshOut->mVertices[base]  = (*i).mPositions[idx];
                    meshOut->mNormals [base]  = (*i).mNormals[idx];

                    if ((*i).mTexCoords.size())
                        meshOut->mTextureCoords[0][base] = (*i).mTexCoords[idx];

                    face.mIndices[a] = base;
                }
            }
        }
    }

    // Copy them to the output array
    pcOut->mNumMeshes = (unsigned int)avOutMeshes.size();
    pcOut->mMeshes = new aiMesh*[pcOut->mNumMeshes]();
    for (unsigned int a = 0; a < pcOut->mNumMeshes;++a) {
        pcOut->mMeshes[a] = avOutMeshes[a];
    }

    // We should have at least one face here
    if (!iFaceCnt) {
        throw DeadlyImportError("No faces loaded. The mesh is empty");
    }
}

// ------------------------------------------------------------------------------------------------
// Add a node to the scenegraph and setup its final transformation
void Discreet3DSImporter::AddNodeToGraph(aiScene* pcSOut,aiNode* pcOut,
    D3DS::Node* pcIn, aiMatrix4x4& /*absTrafo*/)
{
    std::vector<unsigned int> iArray;
    iArray.reserve(3);

    aiMatrix4x4 abs;

    // Find all meshes with the same name as the node
    for (unsigned int a = 0; a < pcSOut->mNumMeshes;++a)
    {
        const D3DS::Mesh* pcMesh = (const D3DS::Mesh*)pcSOut->mMeshes[a]->mColors[0];
        ai_assert(NULL != pcMesh);

        if (pcIn->mName == pcMesh->mName)
            iArray.push_back(a);
    }
    if (!iArray.empty())
    {
        // The matrix should be identical for all meshes with the
        // same name. It HAS to be identical for all meshes .....
        D3DS::Mesh* imesh = ((D3DS::Mesh*)pcSOut->mMeshes[iArray[0]]->mColors[0]);

        // Compute the inverse of the transformation matrix to move the
        // vertices back to their relative and local space
        aiMatrix4x4 mInv = imesh->mMat, mInvTransposed = imesh->mMat;
        mInv.Inverse();mInvTransposed.Transpose();
        aiVector3D pivot = pcIn->vPivot;

        pcOut->mNumMeshes = (unsigned int)iArray.size();
        pcOut->mMeshes = new unsigned int[iArray.size()];
        for (unsigned int i = 0;i < iArray.size();++i)  {
            const unsigned int iIndex = iArray[i];
            aiMesh* const mesh = pcSOut->mMeshes[iIndex];

            if (mesh->mColors[1] == NULL)
            {
                // Transform the vertices back into their local space
                // fixme: consider computing normals after this, so we don't need to transform them
                const aiVector3D* const pvEnd = mesh->mVertices + mesh->mNumVertices;
                aiVector3D* pvCurrent = mesh->mVertices, *t2 = mesh->mNormals;

                for (; pvCurrent != pvEnd; ++pvCurrent, ++t2) {
                    *pvCurrent = mInv * (*pvCurrent);
                    *t2 = mInvTransposed * (*t2);
                }

                // Handle negative transformation matrix determinant -> invert vertex x
                if (imesh->mMat.Determinant() < 0.0f)
                {
                    /* we *must* have normals */
                    for (pvCurrent = mesh->mVertices, t2 = mesh->mNormals; pvCurrent != pvEnd; ++pvCurrent, ++t2) {
                        pvCurrent->x *= -1.f;
                        t2->x *= -1.f;
                    }
                    DefaultLogger::get()->info("3DS: Flipping mesh X-Axis");
                }

                // Handle pivot point
                if (pivot.x || pivot.y || pivot.z)
                {
                    for (pvCurrent = mesh->mVertices; pvCurrent != pvEnd; ++pvCurrent)  {
                        *pvCurrent -= pivot;
                    }
                }

                mesh->mColors[1] = (aiColor4D*)1;
            }
            else
                mesh->mColors[1] = (aiColor4D*)1;

            // Setup the mesh index
            pcOut->mMeshes[i] = iIndex;
        }
    }

    // Setup the name of the node
    // First instance keeps its name otherwise something might break, all others will be postfixed with their instance number
    if (pcIn->mInstanceNumber > 1)
    {
        char tmp[12];
        ASSIMP_itoa10(tmp, pcIn->mInstanceNumber);
        std::string tempStr = pcIn->mName + "_inst_";
        tempStr += tmp;
        pcOut->mName.Set(tempStr);
    }
    else
        pcOut->mName.Set(pcIn->mName);

    // Now build the transformation matrix of the node
    // ROTATION
    if (pcIn->aRotationKeys.size()){

        // FIX to get to Assimp's quaternion conventions
        for (std::vector<aiQuatKey>::iterator it = pcIn->aRotationKeys.begin(); it != pcIn->aRotationKeys.end(); ++it) {
            (*it).mValue.w *= -1.f;
        }

        pcOut->mTransformation = aiMatrix4x4( pcIn->aRotationKeys[0].mValue.GetMatrix() );
    }
    else if (pcIn->aCameraRollKeys.size())
    {
        aiMatrix4x4::RotationZ(AI_DEG_TO_RAD(- pcIn->aCameraRollKeys[0].mValue),
            pcOut->mTransformation);
    }

    // SCALING
    aiMatrix4x4& m = pcOut->mTransformation;
    if (pcIn->aScalingKeys.size())
    {
        const aiVector3D& v = pcIn->aScalingKeys[0].mValue;
        m.a1 *= v.x; m.b1 *= v.x; m.c1 *= v.x;
        m.a2 *= v.y; m.b2 *= v.y; m.c2 *= v.y;
        m.a3 *= v.z; m.b3 *= v.z; m.c3 *= v.z;
    }

    // TRANSLATION
    if (pcIn->aPositionKeys.size())
    {
        const aiVector3D& v = pcIn->aPositionKeys[0].mValue;
        m.a4 += v.x;
        m.b4 += v.y;
        m.c4 += v.z;
    }

    // Generate animation channels for the node
    if (pcIn->aPositionKeys.size()  > 1  || pcIn->aRotationKeys.size()   > 1 ||
        pcIn->aScalingKeys.size()   > 1  || pcIn->aCameraRollKeys.size() > 1 ||
        pcIn->aTargetPositionKeys.size() > 1)
    {
        aiAnimation* anim = pcSOut->mAnimations[0];
        ai_assert(NULL != anim);

        if (pcIn->aCameraRollKeys.size() > 1)
        {
            DefaultLogger::get()->debug("3DS: Converting camera roll track ...");

            // Camera roll keys - in fact they're just rotations
            // around the camera's z axis. The angles are given
            // in degrees (and they're clockwise).
            pcIn->aRotationKeys.resize(pcIn->aCameraRollKeys.size());
            for (unsigned int i = 0; i < pcIn->aCameraRollKeys.size();++i)
            {
                aiQuatKey&  q = pcIn->aRotationKeys[i];
                aiFloatKey& f = pcIn->aCameraRollKeys[i];

                q.mTime  = f.mTime;

                // FIX to get to Assimp quaternion conventions
                q.mValue = aiQuaternion(0.f,0.f,AI_DEG_TO_RAD( /*-*/ f.mValue));
            }
        }
#if 0
        if (pcIn->aTargetPositionKeys.size() > 1)
        {
            DefaultLogger::get()->debug("3DS: Converting target track ...");

            // Camera or spot light - need to convert the separate
            // target position channel to our representation
            TargetAnimationHelper helper;

            if (pcIn->aPositionKeys.empty())
            {
                // We can just pass zero here ...
                helper.SetFixedMainAnimationChannel(aiVector3D());
            }
            else  helper.SetMainAnimationChannel(&pcIn->aPositionKeys);
            helper.SetTargetAnimationChannel(&pcIn->aTargetPositionKeys);

            // Do the conversion
            std::vector<aiVectorKey> distanceTrack;
            helper.Process(&distanceTrack);

            // Now add a new node as child, name it <ourName>.Target
            // and assign the distance track to it. This is that the
            // information where the target is and how it moves is
            // not lost
            D3DS::Node* nd = new D3DS::Node();
            pcIn->push_back(nd);

            nd->mName = pcIn->mName + ".Target";

            aiNodeAnim* nda = anim->mChannels[anim->mNumChannels++] = new aiNodeAnim();
            nda->mNodeName.Set(nd->mName);

            nda->mNumPositionKeys = (unsigned int)distanceTrack.size();
            nda->mPositionKeys = new aiVectorKey[nda->mNumPositionKeys];
            ::memcpy(nda->mPositionKeys,&distanceTrack[0],
                sizeof(aiVectorKey)*nda->mNumPositionKeys);
        }
#endif

        // Cameras or lights define their transformation in their parent node and in the
        // corresponding light or camera chunks. However, we read and process the latter
        // to to be able to return valid cameras/lights even if no scenegraph is given.
        for (unsigned int n = 0; n < pcSOut->mNumCameras;++n)   {
            if (pcSOut->mCameras[n]->mName == pcOut->mName) {
                pcSOut->mCameras[n]->mLookAt = aiVector3D(0.f,0.f,1.f);
            }
        }
        for (unsigned int n = 0; n < pcSOut->mNumLights;++n)    {
            if (pcSOut->mLights[n]->mName == pcOut->mName) {
                pcSOut->mLights[n]->mDirection = aiVector3D(0.f,0.f,1.f);
            }
        }

        // Allocate a new node anim and setup its name
        aiNodeAnim* nda = anim->mChannels[anim->mNumChannels++] = new aiNodeAnim();
        nda->mNodeName.Set(pcIn->mName);

        // POSITION keys
        if (pcIn->aPositionKeys.size()  > 0)
        {
            nda->mNumPositionKeys = (unsigned int)pcIn->aPositionKeys.size();
            nda->mPositionKeys = new aiVectorKey[nda->mNumPositionKeys];
            ::memcpy(nda->mPositionKeys,&pcIn->aPositionKeys[0],
                sizeof(aiVectorKey)*nda->mNumPositionKeys);
        }

        // ROTATION keys
        if (pcIn->aRotationKeys.size()  > 0)
        {
            nda->mNumRotationKeys = (unsigned int)pcIn->aRotationKeys.size();
            nda->mRotationKeys = new aiQuatKey[nda->mNumRotationKeys];

            // Rotations are quaternion offsets
            aiQuaternion abs1;
            for (unsigned int n = 0; n < nda->mNumRotationKeys;++n)
            {
                const aiQuatKey& q = pcIn->aRotationKeys[n];

                abs1 = (n ? abs1 * q.mValue : q.mValue);
                nda->mRotationKeys[n].mTime  = q.mTime;
                nda->mRotationKeys[n].mValue = abs1.Normalize();
            }
        }

        // SCALING keys
        if (pcIn->aScalingKeys.size()  > 0)
        {
            nda->mNumScalingKeys = (unsigned int)pcIn->aScalingKeys.size();
            nda->mScalingKeys = new aiVectorKey[nda->mNumScalingKeys];
            ::memcpy(nda->mScalingKeys,&pcIn->aScalingKeys[0],
                sizeof(aiVectorKey)*nda->mNumScalingKeys);
        }
    }

    // Allocate storage for children
    pcOut->mNumChildren = (unsigned int)pcIn->mChildren.size();
    pcOut->mChildren = new aiNode*[pcIn->mChildren.size()];

    // Recursively process all children
    const unsigned int size = static_cast<unsigned int>(pcIn->mChildren.size());
    for (unsigned int i = 0; i < size;++i)
    {
        pcOut->mChildren[i] = new aiNode();
        pcOut->mChildren[i]->mParent = pcOut;
        AddNodeToGraph(pcSOut,pcOut->mChildren[i],pcIn->mChildren[i],abs);
    }
}

// ------------------------------------------------------------------------------------------------
// Find out how many node animation channels we'll have finally
void CountTracks(D3DS::Node* node, unsigned int& cnt)
{
    //////////////////////////////////////////////////////////////////////////////
    // We will never generate more than one channel for a node, so
    // this is rather easy here.

    if (node->aPositionKeys.size()  > 1  || node->aRotationKeys.size()   > 1   ||
        node->aScalingKeys.size()   > 1  || node->aCameraRollKeys.size() > 1 ||
        node->aTargetPositionKeys.size()  > 1)
    {
        ++cnt;

        // account for the additional channel for the camera/spotlight target position
        if (node->aTargetPositionKeys.size()  > 1)++cnt;
    }

    // Recursively process all children
    for (unsigned int i = 0; i < node->mChildren.size();++i)
        CountTracks(node->mChildren[i],cnt);
}

// ------------------------------------------------------------------------------------------------
// Generate the output node graph
void Discreet3DSImporter::GenerateNodeGraph(aiScene* pcOut)
{
    pcOut->mRootNode = new aiNode();
    if (0 == mRootNode->mChildren.size())
    {
        //////////////////////////////////////////////////////////////////////////////
        // It seems the file is so messed up that it has not even a hierarchy.
        // generate a flat hiearachy which looks like this:
        //
        //                ROOT_NODE
        //                   |
        //   ----------------------------------------
        //   |       |       |            |         |
        // MESH_0  MESH_1  MESH_2  ...  MESH_N    CAMERA_0 ....
        //
        DefaultLogger::get()->warn("No hierarchy information has been found in the file. ");

        pcOut->mRootNode->mNumChildren = pcOut->mNumMeshes +
            static_cast<unsigned int>(mScene->mCameras.size() + mScene->mLights.size());

        pcOut->mRootNode->mChildren = new aiNode* [ pcOut->mRootNode->mNumChildren ];
        pcOut->mRootNode->mName.Set("<3DSDummyRoot>");

        // Build dummy nodes for all meshes
        unsigned int a = 0;
        for (unsigned int i = 0; i < pcOut->mNumMeshes;++i,++a)
        {
            aiNode* pcNode = pcOut->mRootNode->mChildren[a] = new aiNode();
            pcNode->mParent = pcOut->mRootNode;
            pcNode->mMeshes = new unsigned int[1];
            pcNode->mMeshes[0] = i;
            pcNode->mNumMeshes = 1;

            // Build a name for the node
            pcNode->mName.length = ai_snprintf(pcNode->mName.data, MAXLEN, "3DSMesh_%u",i);
        }

        // Build dummy nodes for all cameras
        for (unsigned int i = 0; i < (unsigned int )mScene->mCameras.size();++i,++a)
        {
            aiNode* pcNode = pcOut->mRootNode->mChildren[a] = new aiNode();
            pcNode->mParent = pcOut->mRootNode;

            // Build a name for the node
            pcNode->mName = mScene->mCameras[i]->mName;
        }

        // Build dummy nodes for all lights
        for (unsigned int i = 0; i < (unsigned int )mScene->mLights.size();++i,++a)
        {
            aiNode* pcNode = pcOut->mRootNode->mChildren[a] = new aiNode();
            pcNode->mParent = pcOut->mRootNode;

            // Build a name for the node
            pcNode->mName = mScene->mLights[i]->mName;
        }
    }
    else
    {
        // First of all: find out how many scaling, rotation and translation
        // animation tracks we'll have afterwards
        unsigned int numChannel = 0;
        CountTracks(mRootNode,numChannel);

        if (numChannel)
        {
            // Allocate a primary animation channel
            pcOut->mNumAnimations = 1;
            pcOut->mAnimations    = new aiAnimation*[1];
            aiAnimation* anim     = pcOut->mAnimations[0] = new aiAnimation();

            anim->mName.Set("3DSMasterAnim");

            // Allocate enough storage for all node animation channels,
            // but don't set the mNumChannels member - we'll use it to
            // index into the array
            anim->mChannels = new aiNodeAnim*[numChannel];
        }

        aiMatrix4x4 m;
        AddNodeToGraph(pcOut,  pcOut->mRootNode, mRootNode,m);
    }

    // We used the first and second vertex color set to store some temporary values so we need to cleanup here
    for (unsigned int a = 0; a < pcOut->mNumMeshes; ++a)
    {
        pcOut->mMeshes[a]->mColors[0] = NULL;
        pcOut->mMeshes[a]->mColors[1] = NULL;
    }

    pcOut->mRootNode->mTransformation = aiMatrix4x4(
        1.f,0.f,0.f,0.f,
        0.f,0.f,1.f,0.f,
        0.f,-1.f,0.f,0.f,
        0.f,0.f,0.f,1.f) * pcOut->mRootNode->mTransformation;

    // If the root node is unnamed name it "<3DSRoot>"
    if (::strstr( pcOut->mRootNode->mName.data, "UNNAMED" ) ||
        (pcOut->mRootNode->mName.data[0] == '$' && pcOut->mRootNode->mName.data[1] == '$') )
    {
        pcOut->mRootNode->mName.Set("<3DSRoot>");
    }
}

// ------------------------------------------------------------------------------------------------
// Convert all meshes in the scene and generate the final output scene.
void Discreet3DSImporter::ConvertScene(aiScene* pcOut)
{
    // Allocate enough storage for all output materials
    pcOut->mNumMaterials = (unsigned int)mScene->mMaterials.size();
    pcOut->mMaterials    = new aiMaterial*[pcOut->mNumMaterials];

    //  ... and convert the 3DS materials to aiMaterial's
    for (unsigned int i = 0; i < pcOut->mNumMaterials;++i)
    {
        aiMaterial* pcNew = new aiMaterial();
        ConvertMaterial(mScene->mMaterials[i],*pcNew);
        pcOut->mMaterials[i] = pcNew;
    }

    // Generate the output mesh list
    ConvertMeshes(pcOut);

    // Now copy all light sources to the output scene
    pcOut->mNumLights = (unsigned int)mScene->mLights.size();
    if (pcOut->mNumLights)
    {
        pcOut->mLights = new aiLight*[pcOut->mNumLights];
        ::memcpy(pcOut->mLights,&mScene->mLights[0],sizeof(void*)*pcOut->mNumLights);
    }

    // Now copy all cameras to the output scene
    pcOut->mNumCameras = (unsigned int)mScene->mCameras.size();
    if (pcOut->mNumCameras)
    {
        pcOut->mCameras = new aiCamera*[pcOut->mNumCameras];
        ::memcpy(pcOut->mCameras,&mScene->mCameras[0],sizeof(void*)*pcOut->mNumCameras);
    }
}

#endif // !! ASSIMP_BUILD_NO_3DS_IMPORTER
