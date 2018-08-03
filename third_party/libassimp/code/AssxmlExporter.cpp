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
/** @file  AssxmlExporter.cpp
 *  ASSXML exporter main code
 */
#include <stdarg.h>
#include <assimp/version.h>
#include "ProcessHelper.h"
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Exporter.hpp>

#ifdef ASSIMP_BUILD_NO_OWN_ZLIB
#   include <zlib.h>
#else
#   include <contrib/zlib/zlib.h>
#endif

#include <time.h>
#include <stdio.h>

#ifndef ASSIMP_BUILD_NO_EXPORT
#ifndef ASSIMP_BUILD_NO_ASSXML_EXPORTER

using namespace Assimp;

namespace Assimp    {

namespace AssxmlExport  {

// -----------------------------------------------------------------------------------
static int ioprintf( IOStream * io, const char *format, ... ) {
	using namespace std;
    if ( nullptr == io ) {
        return -1;
    }

    static const int Size = 4096;
    char sz[ Size ];
    ::memset( sz, '\0', Size );
    va_list va;
    va_start( va, format );
    const unsigned int nSize = vsnprintf( sz, Size-1, format, va );
    ai_assert( nSize < Size );
    va_end( va );

    io->Write( sz, sizeof(char), nSize );

    return nSize;
}

// -----------------------------------------------------------------------------------
// Convert a name to standard XML format
static void ConvertName(aiString& out, const aiString& in) {
    out.length = 0;
    for (unsigned int i = 0; i < in.length; ++i)  {
        switch (in.data[i]) {
            case '<':
                out.Append("&lt;");break;
            case '>':
                out.Append("&gt;");break;
            case '&':
                out.Append("&amp;");break;
            case '\"':
                out.Append("&quot;");break;
            case '\'':
                out.Append("&apos;");break;
            default:
                out.data[out.length++] = in.data[i];
        }
    }
    out.data[out.length] = 0;
}

// -----------------------------------------------------------------------------------
// Write a single node as text dump
static void WriteNode(const aiNode* node, IOStream * io, unsigned int depth) {
    char prefix[512];
    for (unsigned int i = 0; i < depth;++i)
        prefix[i] = '\t';
    prefix[depth] = '\0';

    const aiMatrix4x4& m = node->mTransformation;

    aiString name;
    ConvertName(name,node->mName);
    ioprintf(io,"%s<Node name=\"%s\"> \n"
        "%s\t<Matrix4> \n"
        "%s\t\t%0 6f %0 6f %0 6f %0 6f\n"
        "%s\t\t%0 6f %0 6f %0 6f %0 6f\n"
        "%s\t\t%0 6f %0 6f %0 6f %0 6f\n"
        "%s\t\t%0 6f %0 6f %0 6f %0 6f\n"
        "%s\t</Matrix4> \n",
        prefix,name.data,prefix,
        prefix,m.a1,m.a2,m.a3,m.a4,
        prefix,m.b1,m.b2,m.b3,m.b4,
        prefix,m.c1,m.c2,m.c3,m.c4,
        prefix,m.d1,m.d2,m.d3,m.d4,prefix);

    if (node->mNumMeshes) {
        ioprintf(io, "%s\t<MeshRefs num=\"%i\">\n%s\t",
            prefix,node->mNumMeshes,prefix);

        for (unsigned int i = 0; i < node->mNumMeshes;++i) {
            ioprintf(io,"%i ",node->mMeshes[i]);
        }
        ioprintf(io,"\n%s\t</MeshRefs>\n",prefix);
    }

    if (node->mNumChildren) {
        ioprintf(io,"%s\t<NodeList num=\"%i\">\n",
            prefix,node->mNumChildren);

        for (unsigned int i = 0; i < node->mNumChildren;++i) {
            WriteNode(node->mChildren[i],io,depth+2);
        }
        ioprintf(io,"%s\t</NodeList>\n",prefix);
    }
    ioprintf(io,"%s</Node>\n",prefix);
}


// -----------------------------------------------------------------------------------
// Some chuncks of text will need to be encoded for XML
// http://stackoverflow.com/questions/5665231/most-efficient-way-to-escape-xml-html-in-c-string#5665377
static std::string encodeXML(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
            switch(data[pos]) {
                    case '&':  buffer.append("&amp;");              break;
                    case '\"': buffer.append("&quot;");             break;
                    case '\'': buffer.append("&apos;");             break;
                    case '<':  buffer.append("&lt;");                   break;
                    case '>':  buffer.append("&gt;");                   break;
                    default:   buffer.append(&data[pos], 1);    break;
            }
    }
    return buffer;
}

// -----------------------------------------------------------------------------------
// Write a text model dump
static
void WriteDump(const aiScene* scene, IOStream* io, bool shortened) {
    time_t tt = ::time( NULL );
    tm* p     = ::gmtime( &tt );
    ai_assert( nullptr != p );

    // write header
    std::string header(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<ASSIMP format_id=\"1\">\n\n"
        "<!-- XML Model dump produced by assimp dump\n"
        "  Library version: %i.%i.%i\n"
        "  %s\n"
        "-->"
        " \n\n"
        "<Scene flags=\"%d\" postprocessing=\"%i\">\n"
    );

    const unsigned int majorVersion( aiGetVersionMajor() );
    const unsigned int minorVersion( aiGetVersionMinor() );
    const unsigned int rev( aiGetVersionRevision() );
    const char *curtime( asctime( p ) );
    ioprintf( io, header.c_str(), majorVersion, minorVersion, rev, curtime, scene->mFlags, 0 );

    // write the node graph
    WriteNode(scene->mRootNode, io, 0);

#if 0
        // write cameras
    for (unsigned int i = 0; i < scene->mNumCameras;++i) {
        aiCamera* cam  = scene->mCameras[i];
        ConvertName(name,cam->mName);

        // camera header
        ioprintf(io,"\t<Camera parent=\"%s\">\n"
            "\t\t<Vector3 name=\"up\"        > %0 8f %0 8f %0 8f </Vector3>\n"
            "\t\t<Vector3 name=\"lookat\"    > %0 8f %0 8f %0 8f </Vector3>\n"
            "\t\t<Vector3 name=\"pos\"       > %0 8f %0 8f %0 8f </Vector3>\n"
            "\t\t<Float   name=\"fov\"       > %f </Float>\n"
            "\t\t<Float   name=\"aspect\"    > %f </Float>\n"
            "\t\t<Float   name=\"near_clip\" > %f </Float>\n"
            "\t\t<Float   name=\"far_clip\"  > %f </Float>\n"
            "\t</Camera>\n",
            name.data,
            cam->mUp.x,cam->mUp.y,cam->mUp.z,
            cam->mLookAt.x,cam->mLookAt.y,cam->mLookAt.z,
            cam->mPosition.x,cam->mPosition.y,cam->mPosition.z,
            cam->mHorizontalFOV,cam->mAspect,cam->mClipPlaneNear,cam->mClipPlaneFar,i);
    }

    // write lights
    for (unsigned int i = 0; i < scene->mNumLights;++i) {
        aiLight* l  = scene->mLights[i];
        ConvertName(name,l->mName);

        // light header
        ioprintf(io,"\t<Light parent=\"%s\"> type=\"%s\"\n"
            "\t\t<Vector3 name=\"diffuse\"   > %0 8f %0 8f %0 8f </Vector3>\n"
            "\t\t<Vector3 name=\"specular\"  > %0 8f %0 8f %0 8f </Vector3>\n"
            "\t\t<Vector3 name=\"ambient\"   > %0 8f %0 8f %0 8f </Vector3>\n",
            name.data,
            (l->mType == aiLightSource_DIRECTIONAL ? "directional" :
            (l->mType == aiLightSource_POINT ? "point" : "spot" )),
            l->mColorDiffuse.r, l->mColorDiffuse.g, l->mColorDiffuse.b,
            l->mColorSpecular.r,l->mColorSpecular.g,l->mColorSpecular.b,
            l->mColorAmbient.r, l->mColorAmbient.g, l->mColorAmbient.b);

        if (l->mType != aiLightSource_DIRECTIONAL) {
            ioprintf(io,
                "\t\t<Vector3 name=\"pos\"       > %0 8f %0 8f %0 8f </Vector3>\n"
                "\t\t<Float   name=\"atten_cst\" > %f </Float>\n"
                "\t\t<Float   name=\"atten_lin\" > %f </Float>\n"
                "\t\t<Float   name=\"atten_sqr\" > %f </Float>\n",
                l->mPosition.x,l->mPosition.y,l->mPosition.z,
                l->mAttenuationConstant,l->mAttenuationLinear,l->mAttenuationQuadratic);
        }

        if (l->mType != aiLightSource_POINT) {
            ioprintf(io,
                "\t\t<Vector3 name=\"lookat\"    > %0 8f %0 8f %0 8f </Vector3>\n",
                l->mDirection.x,l->mDirection.y,l->mDirection.z);
        }

        if (l->mType == aiLightSource_SPOT) {
            ioprintf(io,
                "\t\t<Float   name=\"cone_out\" > %f </Float>\n"
                "\t\t<Float   name=\"cone_inn\" > %f </Float>\n",
                l->mAngleOuterCone,l->mAngleInnerCone);
        }
        ioprintf(io,"\t</Light>\n");
    }
#endif
    aiString name;

    // write textures
    if (scene->mNumTextures) {
        ioprintf(io,"<TextureList num=\"%i\">\n",scene->mNumTextures);
        for (unsigned int i = 0; i < scene->mNumTextures;++i) {
            aiTexture* tex  = scene->mTextures[i];
            bool compressed = (tex->mHeight == 0);

            // mesh header
            ioprintf(io,"\t<Texture width=\"%i\" height=\"%i\" compressed=\"%s\"> \n",
                (compressed ? -1 : tex->mWidth),(compressed ? -1 : tex->mHeight),
                (compressed ? "true" : "false"));

            if (compressed) {
                ioprintf(io,"\t\t<Data length=\"%i\"> \n",tex->mWidth);

                if (!shortened) {
                    for (unsigned int n = 0; n < tex->mWidth;++n) {
                        ioprintf(io,"\t\t\t%2x",reinterpret_cast<uint8_t*>(tex->pcData)[n]);
                        if (n && !(n % 50)) {
                            ioprintf(io,"\n");
                        }
                    }
                }
            }
            else if (!shortened){
                ioprintf(io,"\t\t<Data length=\"%i\"> \n",tex->mWidth*tex->mHeight*4);

                // const unsigned int width = (unsigned int)std::log10((double)std::max(tex->mHeight,tex->mWidth))+1;
                for (unsigned int y = 0; y < tex->mHeight;++y) {
                    for (unsigned int x = 0; x < tex->mWidth;++x) {
                        aiTexel* tx = tex->pcData + y*tex->mWidth+x;
                        unsigned int r = tx->r,g=tx->g,b=tx->b,a=tx->a;
                        ioprintf(io,"\t\t\t%2x %2x %2x %2x",r,g,b,a);

                        // group by four for readability
                        if ( 0 == ( x + y*tex->mWidth ) % 4 ) {
                            ioprintf( io, "\n" );
                        }
                    }
                }
            }
            ioprintf(io,"\t\t</Data>\n\t</Texture>\n");
        }
        ioprintf(io,"</TextureList>\n");
    }

    // write materials
    if (scene->mNumMaterials) {
        ioprintf(io,"<MaterialList num=\"%i\">\n",scene->mNumMaterials);
        for (unsigned int i = 0; i< scene->mNumMaterials; ++i) {
            const aiMaterial* mat = scene->mMaterials[i];

            ioprintf(io,"\t<Material>\n");
            ioprintf(io,"\t\t<MatPropertyList  num=\"%i\">\n",mat->mNumProperties);
            for (unsigned int n = 0; n < mat->mNumProperties;++n) {

                const aiMaterialProperty* prop = mat->mProperties[n];
                const char* sz = "";
                if (prop->mType == aiPTI_Float) {
                    sz = "float";
                }
                else if (prop->mType == aiPTI_Integer) {
                    sz = "integer";
                }
                else if (prop->mType == aiPTI_String) {
                    sz = "string";
                }
                else if (prop->mType == aiPTI_Buffer) {
                    sz = "binary_buffer";
                }

                ioprintf(io,"\t\t\t<MatProperty key=\"%s\" \n\t\t\ttype=\"%s\" tex_usage=\"%s\" tex_index=\"%i\"",
                    prop->mKey.data, sz,
                    ::TextureTypeToString((aiTextureType)prop->mSemantic),prop->mIndex);

                if (prop->mType == aiPTI_Float) {
                    ioprintf(io," size=\"%i\">\n\t\t\t\t",
                        static_cast<int>(prop->mDataLength/sizeof(float)));

                    for (unsigned int p = 0; p < prop->mDataLength/sizeof(float);++p) {
                        ioprintf(io,"%f ",*((float*)(prop->mData+p*sizeof(float))));
                    }
                }
                else if (prop->mType == aiPTI_Integer) {
                    ioprintf(io," size=\"%i\">\n\t\t\t\t",
                        static_cast<int>(prop->mDataLength/sizeof(int)));

                    for (unsigned int p = 0; p < prop->mDataLength/sizeof(int);++p) {
                        ioprintf(io,"%i ",*((int*)(prop->mData+p*sizeof(int))));
                    }
                }
                else if (prop->mType == aiPTI_Buffer) {
                    ioprintf(io," size=\"%i\">\n\t\t\t\t",
                        static_cast<int>(prop->mDataLength));

                    for (unsigned int p = 0; p < prop->mDataLength;++p) {
                        ioprintf(io,"%2x ",prop->mData[p]);
                        if (p && 0 == p%30) {
                            ioprintf(io,"\n\t\t\t\t");
                        }
                    }
                }
                else if (prop->mType == aiPTI_String) {
                    ioprintf(io,">\n\t\t\t\t\"%s\"",encodeXML(prop->mData+4).c_str() /* skip length */);
                }
                ioprintf(io,"\n\t\t\t</MatProperty>\n");
            }
            ioprintf(io,"\t\t</MatPropertyList>\n");
            ioprintf(io,"\t</Material>\n");
        }
        ioprintf(io,"</MaterialList>\n");
    }

    // write animations
    if (scene->mNumAnimations) {
        ioprintf(io,"<AnimationList num=\"%i\">\n",scene->mNumAnimations);
        for (unsigned int i = 0; i < scene->mNumAnimations;++i) {
            aiAnimation* anim = scene->mAnimations[i];

            // anim header
            ConvertName(name,anim->mName);
            ioprintf(io,"\t<Animation name=\"%s\" duration=\"%e\" tick_cnt=\"%e\">\n",
                name.data, anim->mDuration, anim->mTicksPerSecond);

            // write bone animation channels
            if (anim->mNumChannels) {
                ioprintf(io,"\t\t<NodeAnimList num=\"%i\">\n",anim->mNumChannels);
                for (unsigned int n = 0; n < anim->mNumChannels;++n) {
                    aiNodeAnim* nd = anim->mChannels[n];

                    // node anim header
                    ConvertName(name,nd->mNodeName);
                    ioprintf(io,"\t\t\t<NodeAnim node=\"%s\">\n",name.data);

                    if (!shortened) {
                        // write position keys
                        if (nd->mNumPositionKeys) {
                            ioprintf(io,"\t\t\t\t<PositionKeyList num=\"%i\">\n",nd->mNumPositionKeys);
                            for (unsigned int a = 0; a < nd->mNumPositionKeys;++a) {
                                aiVectorKey* vc = nd->mPositionKeys+a;
                                ioprintf(io,"\t\t\t\t\t<PositionKey time=\"%e\">\n"
                                    "\t\t\t\t\t\t%0 8f %0 8f %0 8f\n\t\t\t\t\t</PositionKey>\n",
                                    vc->mTime,vc->mValue.x,vc->mValue.y,vc->mValue.z);
                            }
                            ioprintf(io,"\t\t\t\t</PositionKeyList>\n");
                        }

                        // write scaling keys
                        if (nd->mNumScalingKeys) {
                            ioprintf(io,"\t\t\t\t<ScalingKeyList num=\"%i\">\n",nd->mNumScalingKeys);
                            for (unsigned int a = 0; a < nd->mNumScalingKeys;++a) {
                                aiVectorKey* vc = nd->mScalingKeys+a;
                                ioprintf(io,"\t\t\t\t\t<ScalingKey time=\"%e\">\n"
                                    "\t\t\t\t\t\t%0 8f %0 8f %0 8f\n\t\t\t\t\t</ScalingKey>\n",
                                    vc->mTime,vc->mValue.x,vc->mValue.y,vc->mValue.z);
                            }
                            ioprintf(io,"\t\t\t\t</ScalingKeyList>\n");
                        }

                        // write rotation keys
                        if (nd->mNumRotationKeys) {
                            ioprintf(io,"\t\t\t\t<RotationKeyList num=\"%i\">\n",nd->mNumRotationKeys);
                            for (unsigned int a = 0; a < nd->mNumRotationKeys;++a) {
                                aiQuatKey* vc = nd->mRotationKeys+a;
                                ioprintf(io,"\t\t\t\t\t<RotationKey time=\"%e\">\n"
                                    "\t\t\t\t\t\t%0 8f %0 8f %0 8f %0 8f\n\t\t\t\t\t</RotationKey>\n",
                                    vc->mTime,vc->mValue.x,vc->mValue.y,vc->mValue.z,vc->mValue.w);
                            }
                            ioprintf(io,"\t\t\t\t</RotationKeyList>\n");
                        }
                    }
                    ioprintf(io,"\t\t\t</NodeAnim>\n");
                }
                ioprintf(io,"\t\t</NodeAnimList>\n");
            }
            ioprintf(io,"\t</Animation>\n");
        }
        ioprintf(io,"</AnimationList>\n");
    }

    // write meshes
    if (scene->mNumMeshes) {
        ioprintf(io,"<MeshList num=\"%i\">\n",scene->mNumMeshes);
        for (unsigned int i = 0; i < scene->mNumMeshes;++i) {
            aiMesh* mesh = scene->mMeshes[i];
            // const unsigned int width = (unsigned int)std::log10((double)mesh->mNumVertices)+1;

            // mesh header
            ioprintf(io,"\t<Mesh types=\"%s %s %s %s\" material_index=\"%i\">\n",
                (mesh->mPrimitiveTypes & aiPrimitiveType_POINT    ? "points"    : ""),
                (mesh->mPrimitiveTypes & aiPrimitiveType_LINE     ? "lines"     : ""),
                (mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE ? "triangles" : ""),
                (mesh->mPrimitiveTypes & aiPrimitiveType_POLYGON  ? "polygons"  : ""),
                mesh->mMaterialIndex);

            // bones
            if (mesh->mNumBones) {
                ioprintf(io,"\t\t<BoneList num=\"%i\">\n",mesh->mNumBones);

                for (unsigned int n = 0; n < mesh->mNumBones;++n) {
                    aiBone* bone = mesh->mBones[n];

                    ConvertName(name,bone->mName);
                    // bone header
                    ioprintf(io,"\t\t\t<Bone name=\"%s\">\n"
                        "\t\t\t\t<Matrix4> \n"
                        "\t\t\t\t\t%0 6f %0 6f %0 6f %0 6f\n"
                        "\t\t\t\t\t%0 6f %0 6f %0 6f %0 6f\n"
                        "\t\t\t\t\t%0 6f %0 6f %0 6f %0 6f\n"
                        "\t\t\t\t\t%0 6f %0 6f %0 6f %0 6f\n"
                        "\t\t\t\t</Matrix4> \n",
                        name.data,
                        bone->mOffsetMatrix.a1,bone->mOffsetMatrix.a2,bone->mOffsetMatrix.a3,bone->mOffsetMatrix.a4,
                        bone->mOffsetMatrix.b1,bone->mOffsetMatrix.b2,bone->mOffsetMatrix.b3,bone->mOffsetMatrix.b4,
                        bone->mOffsetMatrix.c1,bone->mOffsetMatrix.c2,bone->mOffsetMatrix.c3,bone->mOffsetMatrix.c4,
                        bone->mOffsetMatrix.d1,bone->mOffsetMatrix.d2,bone->mOffsetMatrix.d3,bone->mOffsetMatrix.d4);

                    if (!shortened && bone->mNumWeights) {
                        ioprintf(io,"\t\t\t\t<WeightList num=\"%i\">\n",bone->mNumWeights);

                        // bone weights
                        for (unsigned int a = 0; a < bone->mNumWeights;++a) {
                            aiVertexWeight* wght = bone->mWeights+a;

                            ioprintf(io,"\t\t\t\t\t<Weight index=\"%i\">\n\t\t\t\t\t\t%f\n\t\t\t\t\t</Weight>\n",
                                wght->mVertexId,wght->mWeight);
                        }
                        ioprintf(io,"\t\t\t\t</WeightList>\n");
                    }
                    ioprintf(io,"\t\t\t</Bone>\n");
                }
                ioprintf(io,"\t\t</BoneList>\n");
            }

            // faces
            if (!shortened && mesh->mNumFaces) {
                ioprintf(io,"\t\t<FaceList num=\"%i\">\n",mesh->mNumFaces);
                for (unsigned int n = 0; n < mesh->mNumFaces; ++n) {
                    aiFace& f = mesh->mFaces[n];
                    ioprintf(io,"\t\t\t<Face num=\"%i\">\n"
                        "\t\t\t\t",f.mNumIndices);

                    for (unsigned int j = 0; j < f.mNumIndices;++j)
                        ioprintf(io,"%i ",f.mIndices[j]);

                    ioprintf(io,"\n\t\t\t</Face>\n");
                }
                ioprintf(io,"\t\t</FaceList>\n");
            }

            // vertex positions
            if (mesh->HasPositions()) {
                ioprintf(io,"\t\t<Positions num=\"%i\" set=\"0\" num_components=\"3\"> \n",mesh->mNumVertices);
                if (!shortened) {
                    for (unsigned int n = 0; n < mesh->mNumVertices; ++n) {
                        ioprintf(io,"\t\t%0 8f %0 8f %0 8f\n",
                            mesh->mVertices[n].x,
                            mesh->mVertices[n].y,
                            mesh->mVertices[n].z);
                    }
                }
                ioprintf(io,"\t\t</Positions>\n");
            }

            // vertex normals
            if (mesh->HasNormals()) {
                ioprintf(io,"\t\t<Normals num=\"%i\" set=\"0\" num_components=\"3\"> \n",mesh->mNumVertices);
                if (!shortened) {
                    for (unsigned int n = 0; n < mesh->mNumVertices; ++n) {
                        ioprintf(io,"\t\t%0 8f %0 8f %0 8f\n",
                            mesh->mNormals[n].x,
                            mesh->mNormals[n].y,
                            mesh->mNormals[n].z);
                    }
                }
                else {
                }
                ioprintf(io,"\t\t</Normals>\n");
            }

            // vertex tangents and bitangents
            if (mesh->HasTangentsAndBitangents()) {
                ioprintf(io,"\t\t<Tangents num=\"%i\" set=\"0\" num_components=\"3\"> \n",mesh->mNumVertices);
                if (!shortened) {
                    for (unsigned int n = 0; n < mesh->mNumVertices; ++n) {
                        ioprintf(io,"\t\t%0 8f %0 8f %0 8f\n",
                            mesh->mTangents[n].x,
                            mesh->mTangents[n].y,
                            mesh->mTangents[n].z);
                    }
                }
                ioprintf(io,"\t\t</Tangents>\n");

                ioprintf(io,"\t\t<Bitangents num=\"%i\" set=\"0\" num_components=\"3\"> \n",mesh->mNumVertices);
                if (!shortened) {
                    for (unsigned int n = 0; n < mesh->mNumVertices; ++n) {
                        ioprintf(io,"\t\t%0 8f %0 8f %0 8f\n",
                            mesh->mBitangents[n].x,
                            mesh->mBitangents[n].y,
                            mesh->mBitangents[n].z);
                    }
                }
                ioprintf(io,"\t\t</Bitangents>\n");
            }

            // texture coordinates
            for (unsigned int a = 0; a < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++a) {
                if (!mesh->mTextureCoords[a])
                    break;

                ioprintf(io,"\t\t<TextureCoords num=\"%i\" set=\"%i\" num_components=\"%i\"> \n",mesh->mNumVertices,
                    a,mesh->mNumUVComponents[a]);

                if (!shortened) {
                    if (mesh->mNumUVComponents[a] == 3) {
                        for (unsigned int n = 0; n < mesh->mNumVertices; ++n) {
                            ioprintf(io,"\t\t%0 8f %0 8f %0 8f\n",
                                mesh->mTextureCoords[a][n].x,
                                mesh->mTextureCoords[a][n].y,
                                mesh->mTextureCoords[a][n].z);
                        }
                    }
                    else {
                        for (unsigned int n = 0; n < mesh->mNumVertices; ++n) {
                            ioprintf(io,"\t\t%0 8f %0 8f\n",
                                mesh->mTextureCoords[a][n].x,
                                mesh->mTextureCoords[a][n].y);
                        }
                    }
                }
                ioprintf(io,"\t\t</TextureCoords>\n");
            }

            // vertex colors
            for (unsigned int a = 0; a < AI_MAX_NUMBER_OF_COLOR_SETS; ++a) {
                if (!mesh->mColors[a])
                    break;
                ioprintf(io,"\t\t<Colors num=\"%i\" set=\"%i\" num_components=\"4\"> \n",mesh->mNumVertices,a);
                if (!shortened) {
                    for (unsigned int n = 0; n < mesh->mNumVertices; ++n) {
                        ioprintf(io,"\t\t%0 8f %0 8f %0 8f %0 8f\n",
                            mesh->mColors[a][n].r,
                            mesh->mColors[a][n].g,
                            mesh->mColors[a][n].b,
                            mesh->mColors[a][n].a);
                    }
                }
                ioprintf(io,"\t\t</Colors>\n");
            }
            ioprintf(io,"\t</Mesh>\n");
        }
        ioprintf(io,"</MeshList>\n");
    }
    ioprintf(io,"</Scene>\n</ASSIMP>");
}

} // end of namespace AssxmlExport

void ExportSceneAssxml(const char* pFile, IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* /*pProperties*/)
{
    IOStream * out = pIOSystem->Open( pFile, "wt" );
    if (!out) return;

    bool shortened = false;
    AssxmlExport::WriteDump( pScene, out, shortened );

    pIOSystem->Close( out );
}

} // end of namespace Assimp

#endif // ASSIMP_BUILD_NO_ASSXML_EXPORTER
#endif // ASSIMP_BUILD_NO_EXPORT
