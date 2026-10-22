/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team



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

/** @file  B3DImporter.cpp
 *  @brief Implementation of the b3d importer class
 */


#ifndef ASSIMP_BUILD_NO_B3D_IMPORTER

// internal headers
#include "B3D/B3DImporter.h"
#include "PostProcessing/TextureTransform.h"
#include "PostProcessing/ConvertToLHProcess.h"

#include <assimp/StringUtils.h>
#include <assimp/IOSystem.hpp>
#include <assimp/anim.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>

#include <memory>

using namespace Assimp;
using namespace std;

static const aiImporterDesc desc = {
    "BlitzBasic 3D Importer",
    "",
    "",
    "http://www.blitzbasic.com/",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "b3d"
};

// (fixme, Aramis) quick workaround to get rid of all those signed to unsigned warnings
#ifdef _MSC_VER
#	pragma warning (disable: 4018)
#endif

//#define DEBUG_B3D

template<typename T>
void DeleteAllBarePointers(std::vector<T>& x)
{
    for(auto p : x)
    {
        delete p;
    }
}

B3DImporter::~B3DImporter()
{
}

// ------------------------------------------------------------------------------------------------
bool B3DImporter::CanRead( const std::string& pFile, IOSystem* /*pIOHandler*/, bool /*checkSig*/) const{

    size_t pos=pFile.find_last_of( '.' );
    if( pos==string::npos ) return false;

    string ext=pFile.substr( pos+1 );
    if( ext.size()!=3 ) return false;

    return (ext[0]=='b' || ext[0]=='B') && (ext[1]=='3') && (ext[2]=='d' || ext[2]=='D');
}

// ------------------------------------------------------------------------------------------------
// Loader meta information
const aiImporterDesc* B3DImporter::GetInfo () const
{
    return &desc;
}

#ifdef DEBUG_B3D
    extern "C"{ void _stdcall AllocConsole(); }
#endif
// ------------------------------------------------------------------------------------------------
void B3DImporter::InternReadFile( const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler){

#ifdef DEBUG_B3D
    AllocConsole();
    freopen( "conin$","r",stdin );
    freopen( "conout$","w",stdout );
    freopen( "conout$","w",stderr );
    cout<<"Hello world from the B3DImporter!"<<endl;
#endif

    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile));

    // Check whether we can read from the file
    if( file.get() == NULL)
        throw DeadlyImportError( "Failed to open B3D file " + pFile + ".");

    // check whether the .b3d file is large enough to contain
    // at least one chunk.
    size_t fileSize = file->FileSize();
    if( fileSize<8 ) throw DeadlyImportError( "B3D File is too small.");

    _pos=0;
    _buf.resize( fileSize );
    file->Read( &_buf[0],1,fileSize );
    _stack.clear();

    ReadBB3D( pScene );
}

// ------------------------------------------------------------------------------------------------
AI_WONT_RETURN void B3DImporter::Oops(){
    throw DeadlyImportError( "B3D Importer - INTERNAL ERROR" );
}

// ------------------------------------------------------------------------------------------------
AI_WONT_RETURN void B3DImporter::Fail( string str ){
#ifdef DEBUG_B3D
    cout<<"Error in B3D file data: "<<str<<endl;
#endif
    throw DeadlyImportError( "B3D Importer - error in B3D file data: "+str );
}

// ------------------------------------------------------------------------------------------------
int B3DImporter::ReadByte(){
    if( _pos<_buf.size() ) return _buf[_pos++];
    Fail( "EOF" );
    return 0;
}

// ------------------------------------------------------------------------------------------------
int B3DImporter::ReadInt(){
    if( _pos+4<=_buf.size() ){
        int n;
        memcpy(&n, &_buf[_pos], 4);
        _pos+=4;
        return n;
    }
    Fail( "EOF" );
    return 0;
}

// ------------------------------------------------------------------------------------------------
float B3DImporter::ReadFloat(){
    if( _pos+4<=_buf.size() ){
        float n;
        memcpy(&n, &_buf[_pos], 4);
        _pos+=4;
        return n;
    }
    Fail( "EOF" );
    return 0.0f;
}

// ------------------------------------------------------------------------------------------------
aiVector2D B3DImporter::ReadVec2(){
    float x=ReadFloat();
    float y=ReadFloat();
    return aiVector2D( x,y );
}

// ------------------------------------------------------------------------------------------------
aiVector3D B3DImporter::ReadVec3(){
    float x=ReadFloat();
    float y=ReadFloat();
    float z=ReadFloat();
    return aiVector3D( x,y,z );
}

// ------------------------------------------------------------------------------------------------
aiQuaternion B3DImporter::ReadQuat(){
    // (aramis_acg) Fix to adapt the loader to changed quat orientation
    float w=-ReadFloat();
    float x=ReadFloat();
    float y=ReadFloat();
    float z=ReadFloat();
    return aiQuaternion( w,x,y,z );
}

// ------------------------------------------------------------------------------------------------
string B3DImporter::ReadString(){
    string str;
    while( _pos<_buf.size() ){
        char c=(char)ReadByte();
        if( !c ) return str;
        str+=c;
    }
    Fail( "EOF" );
    return string();
}

// ------------------------------------------------------------------------------------------------
string B3DImporter::ReadChunk(){
    string tag;
    for( int i=0;i<4;++i ){
        tag+=char( ReadByte() );
    }
#ifdef DEBUG_B3D
//	cout<<"ReadChunk:"<<tag<<endl;
#endif
    unsigned sz=(unsigned)ReadInt();
    _stack.push_back( _pos+sz );
    return tag;
}

// ------------------------------------------------------------------------------------------------
void B3DImporter::ExitChunk(){
    _pos=_stack.back();
    _stack.pop_back();
}

// ------------------------------------------------------------------------------------------------
unsigned B3DImporter::ChunkSize(){
    return _stack.back()-_pos;
}
// ------------------------------------------------------------------------------------------------

template<class T>
T *B3DImporter::to_array( const vector<T> &v ){
    if( v.empty() ) {
        return 0;
    }
    T *p=new T[ v.size() ];
    for( size_t i=0;i<v.size();++i ){
        p[i]=v[i];
    }
    return p;
}


// ------------------------------------------------------------------------------------------------
template<class T>
T **unique_to_array( vector<std::unique_ptr<T> > &v ){
    if( v.empty() ) {
        return 0;
    }
    T **p = new T*[ v.size() ];
    for( size_t i = 0; i < v.size(); ++i ){
        p[i] = v[i].release();
    }
    return p;
}


// ------------------------------------------------------------------------------------------------
void B3DImporter::ReadTEXS(){
    while( ChunkSize() ){
        string name=ReadString();
        /*int flags=*/ReadInt();
        /*int blend=*/ReadInt();
        /*aiVector2D pos=*/ReadVec2();
        /*aiVector2D scale=*/ReadVec2();
        /*float rot=*/ReadFloat();

        _textures.push_back( name );
    }
}

// ------------------------------------------------------------------------------------------------
void B3DImporter::ReadBRUS(){
    int n_texs=ReadInt();
    if( n_texs<0 || n_texs>8 ){
        Fail( "Bad texture count" );
    }
    while( ChunkSize() ){
        string name=ReadString();
        aiVector3D color=ReadVec3();
        float alpha=ReadFloat();
        float shiny=ReadFloat();
        /*int blend=**/ReadInt();
        int fx=ReadInt();

        std::unique_ptr<aiMaterial> mat(new aiMaterial);

        // Name
        aiString ainame( name );
        mat->AddProperty( &ainame,AI_MATKEY_NAME );

        // Diffuse color
        mat->AddProperty( &color,1,AI_MATKEY_COLOR_DIFFUSE );

        // Opacity
        mat->AddProperty( &alpha,1,AI_MATKEY_OPACITY );

        // Specular color
        aiColor3D speccolor( shiny,shiny,shiny );
        mat->AddProperty( &speccolor,1,AI_MATKEY_COLOR_SPECULAR );

        // Specular power
        float specpow=shiny*128;
        mat->AddProperty( &specpow,1,AI_MATKEY_SHININESS );

        // Double sided
        if( fx & 0x10 ){
            int i=1;
            mat->AddProperty( &i,1,AI_MATKEY_TWOSIDED );
        }

        //Textures
        for( int i=0;i<n_texs;++i ){
            int texid=ReadInt();
            if( texid<-1 || (texid>=0 && texid>=static_cast<int>(_textures.size())) ){
                Fail( "Bad texture id" );
            }
            if( i==0 && texid>=0 ){
                aiString texname( _textures[texid] );
                mat->AddProperty( &texname,AI_MATKEY_TEXTURE_DIFFUSE(0) );
            }
        }
        _materials.emplace_back( std::move(mat) );
    }
}

// ------------------------------------------------------------------------------------------------
void B3DImporter::ReadVRTS(){
    _vflags=ReadInt();
    _tcsets=ReadInt();
    _tcsize=ReadInt();
    if( _tcsets<0 || _tcsets>4 || _tcsize<0 || _tcsize>4 ){
        Fail( "Bad texcoord data" );
    }

    int sz=12+(_vflags&1?12:0)+(_vflags&2?16:0)+(_tcsets*_tcsize*4);
    int n_verts=ChunkSize()/sz;

    int v0=static_cast<int>(_vertices.size());
    _vertices.resize( v0+n_verts );

    for( int i=0;i<n_verts;++i ){
        Vertex &v=_vertices[v0+i];

        memset( v.bones,0,sizeof(v.bones) );
        memset( v.weights,0,sizeof(v.weights) );

        v.vertex=ReadVec3();

        if( _vflags & 1 ) v.normal=ReadVec3();

        if( _vflags & 2 ) ReadQuat();	//skip v 4bytes...

        for( int i=0;i<_tcsets;++i ){
            float t[4]={0,0,0,0};
            for( int j=0;j<_tcsize;++j ){
                t[j]=ReadFloat();
            }
            t[1]=1-t[1];
            if( !i ) v.texcoords=aiVector3D( t[0],t[1],t[2] );
        }
    }
}

// ------------------------------------------------------------------------------------------------
void B3DImporter::ReadTRIS( int v0 ){
    int matid=ReadInt();
    if( matid==-1 ){
        matid=0;
    }else if( matid<0 || matid>=(int)_materials.size() ){
#ifdef DEBUG_B3D
        cout<<"material id="<<matid<<endl;
#endif
        Fail( "Bad material id" );
    }

    std::unique_ptr<aiMesh> mesh(new aiMesh);

    mesh->mMaterialIndex=matid;
    mesh->mNumFaces=0;
    mesh->mPrimitiveTypes=aiPrimitiveType_TRIANGLE;

    int n_tris=ChunkSize()/12;
    aiFace *face=mesh->mFaces=new aiFace[n_tris];

    for( int i=0;i<n_tris;++i ){
        int i0=ReadInt()+v0;
        int i1=ReadInt()+v0;
        int i2=ReadInt()+v0;
        if( i0<0 || i0>=(int)_vertices.size() || i1<0 || i1>=(int)_vertices.size() || i2<0 || i2>=(int)_vertices.size() ){
#ifdef DEBUG_B3D
            cout<<"Bad triangle index: i0="<<i0<<", i1="<<i1<<", i2="<<i2<<endl;
#endif
            Fail( "Bad triangle index" );
            continue;
        }
        face->mNumIndices=3;
        face->mIndices=new unsigned[3];
        face->mIndices[0]=i0;
        face->mIndices[1]=i1;
        face->mIndices[2]=i2;
        ++mesh->mNumFaces;
        ++face;
    }

    _meshes.emplace_back( std::move(mesh) );
}

// ------------------------------------------------------------------------------------------------
void B3DImporter::ReadMESH(){
    /*int matid=*/ReadInt();

    int v0= static_cast<int>(_vertices.size());

    while( ChunkSize() ){
        string t=ReadChunk();
        if( t=="VRTS" ){
            ReadVRTS();
        }else if( t=="TRIS" ){
            ReadTRIS( v0 );
        }
        ExitChunk();
    }
}

// ------------------------------------------------------------------------------------------------
void B3DImporter::ReadBONE( int id ){
    while( ChunkSize() ){
        int vertex=ReadInt();
        float weight=ReadFloat();
        if( vertex<0 || vertex>=(int)_vertices.size() ){
            Fail( "Bad vertex index" );
        }

        Vertex &v=_vertices[vertex];
        int i;
        for( i=0;i<4;++i ){
            if( !v.weights[i] ){
                v.bones[i]=id;
                v.weights[i]=weight;
                break;
            }
        }
#ifdef DEBUG_B3D
        if( i==4 ){
            cout<<"Too many bone weights"<<endl;
        }
#endif
    }
}

// ------------------------------------------------------------------------------------------------
void B3DImporter::ReadKEYS( aiNodeAnim *nodeAnim ){
    vector<aiVectorKey> trans,scale;
    vector<aiQuatKey> rot;
    int flags=ReadInt();
    while( ChunkSize() ){
        int frame=ReadInt();
        if( flags & 1 ){
            trans.push_back( aiVectorKey( frame,ReadVec3() ) );
        }
        if( flags & 2 ){
            scale.push_back( aiVectorKey( frame,ReadVec3() ) );
        }
        if( flags & 4 ){
            rot.push_back( aiQuatKey( frame,ReadQuat() ) );
        }
    }

    if( flags & 1 ){
        nodeAnim->mNumPositionKeys=static_cast<unsigned int>(trans.size());
        nodeAnim->mPositionKeys=to_array( trans );
    }

    if( flags & 2 ){
        nodeAnim->mNumScalingKeys=static_cast<unsigned int>(scale.size());
        nodeAnim->mScalingKeys=to_array( scale );
    }

    if( flags & 4 ){
        nodeAnim->mNumRotationKeys=static_cast<unsigned int>(rot.size());
        nodeAnim->mRotationKeys=to_array( rot );
    }
}

// ------------------------------------------------------------------------------------------------
void B3DImporter::ReadANIM(){
    /*int flags=*/ReadInt();
    int frames=ReadInt();
    float fps=ReadFloat();

    std::unique_ptr<aiAnimation> anim(new aiAnimation);

    anim->mDuration=frames;
    anim->mTicksPerSecond=fps;
    _animations.emplace_back( std::move(anim) );
}

// ------------------------------------------------------------------------------------------------
aiNode *B3DImporter::ReadNODE( aiNode *parent ){

    string name=ReadString();
    aiVector3D t=ReadVec3();
    aiVector3D s=ReadVec3();
    aiQuaternion r=ReadQuat();

    aiMatrix4x4 trans,scale,rot;

    aiMatrix4x4::Translation( t,trans );
    aiMatrix4x4::Scaling( s,scale );
    rot=aiMatrix4x4( r.GetMatrix() );

    aiMatrix4x4 tform=trans * rot * scale;

    int nodeid=static_cast<int>(_nodes.size());

    aiNode *node=new aiNode( name );
    _nodes.push_back( node );

    node->mParent=parent;
    node->mTransformation=tform;

    std::unique_ptr<aiNodeAnim> nodeAnim;
    vector<unsigned> meshes;
    vector<aiNode*> children;

    while( ChunkSize() ){
        string t=ReadChunk();
        if( t=="MESH" ){
            unsigned int n= static_cast<unsigned int>(_meshes.size());
            ReadMESH();
            for( unsigned int i=n;i<static_cast<unsigned int>(_meshes.size());++i ){
                meshes.push_back( i );
            }
        }else if( t=="BONE" ){
            ReadBONE( nodeid );
        }else if( t=="ANIM" ){
            ReadANIM();
        }else if( t=="KEYS" ){
            if( !nodeAnim ){
                nodeAnim.reset(new aiNodeAnim);
                nodeAnim->mNodeName=node->mName;
            }
            ReadKEYS( nodeAnim.get() );
        }else if( t=="NODE" ){
            aiNode *child=ReadNODE( node );
            children.push_back( child );
        }
        ExitChunk();
    }

    if (nodeAnim) {
        _nodeAnims.emplace_back( std::move(nodeAnim) );
    }

    node->mNumMeshes= static_cast<unsigned int>(meshes.size());
    node->mMeshes=to_array( meshes );

    node->mNumChildren=static_cast<unsigned int>(children.size());
    node->mChildren=to_array( children );

    return node;
}

// ------------------------------------------------------------------------------------------------
void B3DImporter::ReadBB3D( aiScene *scene ){

    _textures.clear();

    _materials.clear();

    _vertices.clear();

    _meshes.clear();

    DeleteAllBarePointers(_nodes);
    _nodes.clear();

    _nodeAnims.clear();

    _animations.clear();

    string t=ReadChunk();
    if( t=="BB3D" ){
        int version=ReadInt();

        if (!DefaultLogger::isNullLogger()) {
            char dmp[128];
            ai_snprintf(dmp, 128, "B3D file format version: %i",version);
            ASSIMP_LOG_INFO(dmp);
        }

        while( ChunkSize() ){
            string t=ReadChunk();
            if( t=="TEXS" ){
                ReadTEXS();
            }else if( t=="BRUS" ){
                ReadBRUS();
            }else if( t=="NODE" ){
                ReadNODE( 0 );
            }
            ExitChunk();
        }
    }
    ExitChunk();

    if( !_nodes.size() ) Fail( "No nodes" );

    if( !_meshes.size() ) Fail( "No meshes" );

    //Fix nodes/meshes/bones
    for(size_t i=0;i<_nodes.size();++i ){
        aiNode *node=_nodes[i];

        for( size_t j=0;j<node->mNumMeshes;++j ){
            aiMesh *mesh = _meshes[node->mMeshes[j]].get();

            int n_tris=mesh->mNumFaces;
            int n_verts=mesh->mNumVertices=n_tris * 3;

            aiVector3D *mv=mesh->mVertices=new aiVector3D[ n_verts ],*mn=0,*mc=0;
            if( _vflags & 1 ) mn=mesh->mNormals=new aiVector3D[ n_verts ];
            if( _tcsets ) mc=mesh->mTextureCoords[0]=new aiVector3D[ n_verts ];

            aiFace *face=mesh->mFaces;

            vector< vector<aiVertexWeight> > vweights( _nodes.size() );

            for( int i=0;i<n_verts;i+=3 ){
                for( int j=0;j<3;++j ){
                    Vertex &v=_vertices[face->mIndices[j]];

                    *mv++=v.vertex;
                    if( mn ) *mn++=v.normal;
                    if( mc ) *mc++=v.texcoords;

                    face->mIndices[j]=i+j;

                    for( int k=0;k<4;++k ){
                        if( !v.weights[k] ) break;

                        int bone=v.bones[k];
                        float weight=v.weights[k];

                        vweights[bone].push_back( aiVertexWeight(i+j,weight) );
                    }
                }
                ++face;
            }

            vector<aiBone*> bones;
            for(size_t i=0;i<vweights.size();++i ){
                vector<aiVertexWeight> &weights=vweights[i];
                if( !weights.size() ) continue;

                aiBone *bone=new aiBone;
                bones.push_back( bone );

                aiNode *bnode=_nodes[i];

                bone->mName=bnode->mName;
                bone->mNumWeights= static_cast<unsigned int>(weights.size());
                bone->mWeights=to_array( weights );

                aiMatrix4x4 mat=bnode->mTransformation;
                while( bnode->mParent ){
                    bnode=bnode->mParent;
                    mat=bnode->mTransformation * mat;
                }
                bone->mOffsetMatrix=mat.Inverse();
            }
            mesh->mNumBones= static_cast<unsigned int>(bones.size());
            mesh->mBones=to_array( bones );
        }
    }

    //nodes
    scene->mRootNode=_nodes[0];
    _nodes.clear();  // node ownership now belongs to scene

    //material
    if( !_materials.size() ){
        _materials.emplace_back( std::unique_ptr<aiMaterial>(new aiMaterial) );
    }
    scene->mNumMaterials= static_cast<unsigned int>(_materials.size());
    scene->mMaterials = unique_to_array( _materials );

    //meshes
    scene->mNumMeshes= static_cast<unsigned int>(_meshes.size());
    scene->mMeshes = unique_to_array( _meshes );

    //animations
    if( _animations.size()==1 && _nodeAnims.size() ){

        aiAnimation *anim = _animations.back().get();
        anim->mNumChannels=static_cast<unsigned int>(_nodeAnims.size());
        anim->mChannels = unique_to_array( _nodeAnims );

        scene->mNumAnimations=static_cast<unsigned int>(_animations.size());
        scene->mAnimations=unique_to_array( _animations );
    }

    // convert to RH
    MakeLeftHandedProcess makeleft;
    makeleft.Execute( scene );

    FlipWindingOrderProcess flip;
    flip.Execute( scene );
}

#endif // !! ASSIMP_BUILD_NO_B3D_IMPORTER
