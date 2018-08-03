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

/** @file  FBXConverter.cpp
 *  @brief Implementation of the FBX DOM -> aiScene converter
 */

#ifndef ASSIMP_BUILD_NO_FBX_IMPORTER

#include "FBXConverter.h"
#include "FBXParser.h"
#include "FBXMeshGeometry.h"
#include "FBXDocument.h"
#include "FBXUtil.h"
#include "FBXProperties.h"
#include "FBXImporter.h"
#include "StringComparison.h"

#include <assimp/scene.h>

#include <tuple>
#include <memory>
#include <iterator>
#include <vector>

namespace Assimp {
namespace FBX {

using namespace Util;


#define MAGIC_NODE_TAG "_$AssimpFbx$"

#define CONVERT_FBX_TIME(time) static_cast<double>(time) / 46186158000L

// XXX vc9's debugger won't step into anonymous namespaces
//namespace {

/** Dummy class to encapsulate the conversion process */
class Converter
{
public:
    /**
     *  The different parts that make up the final local transformation of a fbx-node
     */
    enum TransformationComp
    {
        TransformationComp_Translation = 0,
        TransformationComp_RotationOffset,
        TransformationComp_RotationPivot,
        TransformationComp_PreRotation,
        TransformationComp_Rotation,
        TransformationComp_PostRotation,
        TransformationComp_RotationPivotInverse,
        TransformationComp_ScalingOffset,
        TransformationComp_ScalingPivot,
        TransformationComp_Scaling,
        TransformationComp_ScalingPivotInverse,
        TransformationComp_GeometricTranslation,
        TransformationComp_GeometricRotation,
        TransformationComp_GeometricScaling,

        TransformationComp_MAXIMUM
    };

public:
    Converter( aiScene* out, const Document& doc );
    ~Converter();

private:
    // ------------------------------------------------------------------------------------------------
    // find scene root and trigger recursive scene conversion
    void ConvertRootNode();

    // ------------------------------------------------------------------------------------------------
    // collect and assign child nodes
    void ConvertNodes( uint64_t id, aiNode& parent, const aiMatrix4x4& parent_transform = aiMatrix4x4() );

    // ------------------------------------------------------------------------------------------------
    void ConvertLights( const Model& model );

    // ------------------------------------------------------------------------------------------------
    void ConvertCameras( const Model& model );

    // ------------------------------------------------------------------------------------------------
    void ConvertLight( const Model& model, const Light& light );

    // ------------------------------------------------------------------------------------------------
    void ConvertCamera( const Model& model, const Camera& cam );

    // ------------------------------------------------------------------------------------------------
    // this returns unified names usable within assimp identifiers (i.e. no space characters -
    // while these would be allowed, they are a potential trouble spot so better not use them).
    const char* NameTransformationComp( TransformationComp comp );

    // ------------------------------------------------------------------------------------------------
    // note: this returns the REAL fbx property names
    const char* NameTransformationCompProperty( TransformationComp comp );

    // ------------------------------------------------------------------------------------------------
    aiVector3D TransformationCompDefaultValue( TransformationComp comp );

    // ------------------------------------------------------------------------------------------------
    void GetRotationMatrix( Model::RotOrder mode, const aiVector3D& rotation, aiMatrix4x4& out );
    // ------------------------------------------------------------------------------------------------
    /**
     *  checks if a node has more than just scaling, rotation and translation components
     */
    bool NeedsComplexTransformationChain( const Model& model );

    // ------------------------------------------------------------------------------------------------
    // note: name must be a FixNodeName() result
    std::string NameTransformationChainNode( const std::string& name, TransformationComp comp );

    // ------------------------------------------------------------------------------------------------
    /**
     *  note: memory for output_nodes will be managed by the caller
     */
    void GenerateTransformationNodeChain( const Model& model, std::vector<aiNode*>& output_nodes );

    // ------------------------------------------------------------------------------------------------
    void SetupNodeMetadata( const Model& model, aiNode& nd );

    // ------------------------------------------------------------------------------------------------
    void ConvertModel( const Model& model, aiNode& nd, const aiMatrix4x4& node_global_transform );

    // ------------------------------------------------------------------------------------------------
    // MeshGeometry -> aiMesh, return mesh index + 1 or 0 if the conversion failed
    std::vector<unsigned int> ConvertMesh( const MeshGeometry& mesh, const Model& model,
        const aiMatrix4x4& node_global_transform );

    // ------------------------------------------------------------------------------------------------
    aiMesh* SetupEmptyMesh( const MeshGeometry& mesh );

    // ------------------------------------------------------------------------------------------------
    unsigned int ConvertMeshSingleMaterial( const MeshGeometry& mesh, const Model& model,
        const aiMatrix4x4& node_global_transform );

    // ------------------------------------------------------------------------------------------------
    std::vector<unsigned int> ConvertMeshMultiMaterial( const MeshGeometry& mesh, const Model& model,
        const aiMatrix4x4& node_global_transform );

    // ------------------------------------------------------------------------------------------------
    unsigned int ConvertMeshMultiMaterial( const MeshGeometry& mesh, const Model& model,
        MatIndexArray::value_type index,
        const aiMatrix4x4& node_global_transform );

    // ------------------------------------------------------------------------------------------------
    static const unsigned int NO_MATERIAL_SEPARATION = /* std::numeric_limits<unsigned int>::max() */
        static_cast<unsigned int>(-1);

    // ------------------------------------------------------------------------------------------------
    /**
     *  - if materialIndex == NO_MATERIAL_SEPARATION, materials are not taken into
     *    account when determining which weights to include.
     *  - outputVertStartIndices is only used when a material index is specified, it gives for
     *    each output vertex the DOM index it maps to.
     */
    void ConvertWeights( aiMesh* out, const Model& model, const MeshGeometry& geo,
        const aiMatrix4x4& node_global_transform = aiMatrix4x4(),
        unsigned int materialIndex = NO_MATERIAL_SEPARATION,
        std::vector<unsigned int>* outputVertStartIndices = NULL );

    // ------------------------------------------------------------------------------------------------
    void ConvertCluster( std::vector<aiBone*>& bones, const Model& /*model*/, const Cluster& cl,
        std::vector<size_t>& out_indices,
        std::vector<size_t>& index_out_indices,
        std::vector<size_t>& count_out_indices,
        const aiMatrix4x4& node_global_transform );

    // ------------------------------------------------------------------------------------------------
    void ConvertMaterialForMesh( aiMesh* out, const Model& model, const MeshGeometry& geo,
        MatIndexArray::value_type materialIndex );

    // ------------------------------------------------------------------------------------------------
    unsigned int GetDefaultMaterial();


    // ------------------------------------------------------------------------------------------------
    // Material -> aiMaterial
    unsigned int ConvertMaterial( const Material& material, const MeshGeometry* const mesh );

    // ------------------------------------------------------------------------------------------------
    // Video -> aiTexture
    unsigned int ConvertVideo( const Video& video );

    // ------------------------------------------------------------------------------------------------
    void TrySetTextureProperties( aiMaterial* out_mat, const TextureMap& textures,
        const std::string& propName,
        aiTextureType target, const MeshGeometry* const mesh );

    // ------------------------------------------------------------------------------------------------
    void TrySetTextureProperties( aiMaterial* out_mat, const LayeredTextureMap& layeredTextures,
        const std::string& propName,
        aiTextureType target, const MeshGeometry* const mesh );

    // ------------------------------------------------------------------------------------------------
    void SetTextureProperties( aiMaterial* out_mat, const TextureMap& textures, const MeshGeometry* const mesh );

    // ------------------------------------------------------------------------------------------------
    void SetTextureProperties( aiMaterial* out_mat, const LayeredTextureMap& layeredTextures, const MeshGeometry* const mesh );

    // ------------------------------------------------------------------------------------------------
    aiColor3D GetColorPropertyFromMaterial( const PropertyTable& props, const std::string& baseName,
        bool& result );

    // ------------------------------------------------------------------------------------------------
    void SetShadingPropertiesCommon( aiMaterial* out_mat, const PropertyTable& props );

    // ------------------------------------------------------------------------------------------------
    // get the number of fps for a FrameRate enumerated value
    static double FrameRateToDouble( FileGlobalSettings::FrameRate fp, double customFPSVal = -1.0 );

    // ------------------------------------------------------------------------------------------------
    // convert animation data to aiAnimation et al
    void ConvertAnimations();

    // ------------------------------------------------------------------------------------------------
    // rename a node already partially converted. fixed_name is a string previously returned by
    // FixNodeName, new_name specifies the string FixNodeName should return on all further invocations
    // which would previously have returned the old value.
    //
    // this also updates names in node animations, cameras and light sources and is thus slow.
    //
    // NOTE: the caller is responsible for ensuring that the new name is unique and does
    // not collide with any other identifiers. The best way to ensure this is to only
    // append to the old name, which is guaranteed to match these requirements.
    void RenameNode( const std::string& fixed_name, const std::string& new_name );

    // ------------------------------------------------------------------------------------------------
    // takes a fbx node name and returns the identifier to be used in the assimp output scene.
    // the function is guaranteed to provide consistent results over multiple invocations
    // UNLESS RenameNode() is called for a particular node name.
    std::string FixNodeName( const std::string& name );

    typedef std::map<const AnimationCurveNode*, const AnimationLayer*> LayerMap;

    // XXX: better use multi_map ..
    typedef std::map<std::string, std::vector<const AnimationCurveNode*> > NodeMap;


    // ------------------------------------------------------------------------------------------------
    void ConvertAnimationStack( const AnimationStack& st );

    // ------------------------------------------------------------------------------------------------
    void GenerateNodeAnimations( std::vector<aiNodeAnim*>& node_anims,
        const std::string& fixed_name,
        const std::vector<const AnimationCurveNode*>& curves,
        const LayerMap& layer_map,
        int64_t start, int64_t stop,
        double& max_time,
        double& min_time );

    // ------------------------------------------------------------------------------------------------
    bool IsRedundantAnimationData( const Model& target,
        TransformationComp comp,
        const std::vector<const AnimationCurveNode*>& curves );

    // ------------------------------------------------------------------------------------------------
    aiNodeAnim* GenerateRotationNodeAnim( const std::string& name,
        const Model& target,
        const std::vector<const AnimationCurveNode*>& curves,
        const LayerMap& layer_map,
        int64_t start, int64_t stop,
        double& max_time,
        double& min_time );

    // ------------------------------------------------------------------------------------------------
    aiNodeAnim* GenerateScalingNodeAnim( const std::string& name,
        const Model& /*target*/,
        const std::vector<const AnimationCurveNode*>& curves,
        const LayerMap& layer_map,
        int64_t start, int64_t stop,
        double& max_time,
        double& min_time );

    // ------------------------------------------------------------------------------------------------
    aiNodeAnim* GenerateTranslationNodeAnim( const std::string& name,
        const Model& /*target*/,
        const std::vector<const AnimationCurveNode*>& curves,
        const LayerMap& layer_map,
        int64_t start, int64_t stop,
        double& max_time,
        double& min_time,
        bool inverse = false );

    // ------------------------------------------------------------------------------------------------
    // generate node anim, extracting only Rotation, Scaling and Translation from the given chain
    aiNodeAnim* GenerateSimpleNodeAnim( const std::string& name,
        const Model& target,
        NodeMap::const_iterator chain[ TransformationComp_MAXIMUM ],
        NodeMap::const_iterator iter_end,
        const LayerMap& layer_map,
        int64_t start, int64_t stop,
        double& max_time,
        double& min_time,
        bool reverse_order = false );

    // key (time), value, mapto (component index)
    typedef std::tuple<std::shared_ptr<KeyTimeList>, std::shared_ptr<KeyValueList>, unsigned int > KeyFrameList;
    typedef std::vector<KeyFrameList> KeyFrameListList;

    // ------------------------------------------------------------------------------------------------
    KeyFrameListList GetKeyframeList( const std::vector<const AnimationCurveNode*>& nodes, int64_t start, int64_t stop );

    // ------------------------------------------------------------------------------------------------
    KeyTimeList GetKeyTimeList( const KeyFrameListList& inputs );

    // ------------------------------------------------------------------------------------------------
    void InterpolateKeys( aiVectorKey* valOut, const KeyTimeList& keys, const KeyFrameListList& inputs,
        const aiVector3D& def_value,
        double& max_time,
        double& min_time );

    // ------------------------------------------------------------------------------------------------
    void InterpolateKeys( aiQuatKey* valOut, const KeyTimeList& keys, const KeyFrameListList& inputs,
        const aiVector3D& def_value,
        double& maxTime,
        double& minTime,
        Model::RotOrder order );

    // ------------------------------------------------------------------------------------------------
    void ConvertTransformOrder_TRStoSRT( aiQuatKey* out_quat, aiVectorKey* out_scale,
        aiVectorKey* out_translation,
        const KeyFrameListList& scaling,
        const KeyFrameListList& translation,
        const KeyFrameListList& rotation,
        const KeyTimeList& times,
        double& maxTime,
        double& minTime,
        Model::RotOrder order,
        const aiVector3D& def_scale,
        const aiVector3D& def_translate,
        const aiVector3D& def_rotation );

    // ------------------------------------------------------------------------------------------------
    // euler xyz -> quat
    aiQuaternion EulerToQuaternion( const aiVector3D& rot, Model::RotOrder order );

    // ------------------------------------------------------------------------------------------------
    void ConvertScaleKeys( aiNodeAnim* na, const std::vector<const AnimationCurveNode*>& nodes, const LayerMap& /*layers*/,
        int64_t start, int64_t stop,
        double& maxTime,
        double& minTime );

    // ------------------------------------------------------------------------------------------------
    void ConvertTranslationKeys( aiNodeAnim* na, const std::vector<const AnimationCurveNode*>& nodes,
        const LayerMap& /*layers*/,
        int64_t start, int64_t stop,
        double& maxTime,
        double& minTime );

    // ------------------------------------------------------------------------------------------------
    void ConvertRotationKeys( aiNodeAnim* na, const std::vector<const AnimationCurveNode*>& nodes,
        const LayerMap& /*layers*/,
        int64_t start, int64_t stop,
        double& maxTime,
        double& minTime,
        Model::RotOrder order );

    // ------------------------------------------------------------------------------------------------
    // copy generated meshes, animations, lights, cameras and textures to the output scene
    void TransferDataToScene();

private:

    // 0: not assigned yet, others: index is value - 1
    unsigned int defaultMaterialIndex;

    std::vector<aiMesh*> meshes;
    std::vector<aiMaterial*> materials;
    std::vector<aiAnimation*> animations;
    std::vector<aiLight*> lights;
    std::vector<aiCamera*> cameras;
    std::vector<aiTexture*> textures;

    typedef std::map<const Material*, unsigned int> MaterialMap;
    MaterialMap materials_converted;

    typedef std::map<const Video*, unsigned int> VideoMap;
    VideoMap textures_converted;

    typedef std::map<const Geometry*, std::vector<unsigned int> > MeshMap;
    MeshMap meshes_converted;

    // fixed node name -> which trafo chain components have animations?
    typedef std::map<std::string, unsigned int> NodeAnimBitMap;
    NodeAnimBitMap node_anim_chain_bits;

    // name -> has had its prefix_stripped?
    typedef std::map<std::string, bool> NodeNameMap;
    NodeNameMap node_names;

    typedef std::map<std::string, std::string> NameNameMap;
    NameNameMap renamed_nodes;

    double anim_fps;

    aiScene* const out;
    const FBX::Document& doc;

	bool FindTextureIndexByFilename(const Video& video, unsigned int& index) {
		index = 0;
		const char* videoFileName = video.FileName().c_str();
		for (auto texture = textures_converted.begin(); texture != textures_converted.end(); ++texture) {
			if (!strcmp(texture->first->FileName().c_str(), videoFileName)) {
                index = texture->second;
				return true;
			}
		}
		return false;
	}
};

Converter::Converter( aiScene* out, const Document& doc )
    : defaultMaterialIndex()
    , out( out )
    , doc( doc )
{
    // animations need to be converted first since this will
    // populate the node_anim_chain_bits map, which is needed
    // to determine which nodes need to be generated.
    ConvertAnimations();
    ConvertRootNode();

    if ( doc.Settings().readAllMaterials ) {
        // unfortunately this means we have to evaluate all objects
        for( const ObjectMap::value_type& v : doc.Objects() ) {

            const Object* ob = v.second->Get();
            if ( !ob ) {
                continue;
            }

            const Material* mat = dynamic_cast<const Material*>( ob );
            if ( mat ) {

                if ( materials_converted.find( mat ) == materials_converted.end() ) {
                    ConvertMaterial( *mat, 0 );
                }
            }
        }
    }

    TransferDataToScene();

    // if we didn't read any meshes set the AI_SCENE_FLAGS_INCOMPLETE
    // to make sure the scene passes assimp's validation. FBX files
    // need not contain geometry (i.e. camera animations, raw armatures).
    if ( out->mNumMeshes == 0 ) {
        out->mFlags |= AI_SCENE_FLAGS_INCOMPLETE;
    }
}


Converter::~Converter()
{
    std::for_each( meshes.begin(), meshes.end(), Util::delete_fun<aiMesh>() );
    std::for_each( materials.begin(), materials.end(), Util::delete_fun<aiMaterial>() );
    std::for_each( animations.begin(), animations.end(), Util::delete_fun<aiAnimation>() );
    std::for_each( lights.begin(), lights.end(), Util::delete_fun<aiLight>() );
    std::for_each( cameras.begin(), cameras.end(), Util::delete_fun<aiCamera>() );
    std::for_each( textures.begin(), textures.end(), Util::delete_fun<aiTexture>() );
}

void Converter::ConvertRootNode()
{
    out->mRootNode = new aiNode();
    out->mRootNode->mName.Set( "RootNode" );

    // root has ID 0
    ConvertNodes( 0L, *out->mRootNode );
}


void Converter::ConvertNodes( uint64_t id, aiNode& parent, const aiMatrix4x4& parent_transform )
{
    const std::vector<const Connection*>& conns = doc.GetConnectionsByDestinationSequenced( id, "Model" );

    std::vector<aiNode*> nodes;
    nodes.reserve( conns.size() );

    std::vector<aiNode*> nodes_chain;

    try {
        for( const Connection* con : conns ) {

            // ignore object-property links
            if ( con->PropertyName().length() ) {
                continue;
            }

            const Object* const object = con->SourceObject();
            if ( !object ) {
                FBXImporter::LogWarn( "failed to convert source object for Model link" );
                continue;
            }

            const Model* const model = dynamic_cast<const Model*>( object );

            if ( model ) {
                nodes_chain.clear();

                aiMatrix4x4 new_abs_transform = parent_transform;

                // even though there is only a single input node, the design of
                // assimp (or rather: the complicated transformation chain that
                // is employed by fbx) means that we may need multiple aiNode's
                // to represent a fbx node's transformation.
                GenerateTransformationNodeChain( *model, nodes_chain );

                ai_assert( nodes_chain.size() );

                const std::string& original_name = FixNodeName( model->Name() );

                // check if any of the nodes in the chain has the name the fbx node
                // is supposed to have. If there is none, add another node to
                // preserve the name - people might have scripts etc. that rely
                // on specific node names.
                aiNode* name_carrier = NULL;
                for( aiNode* prenode : nodes_chain ) {
                    if ( !strcmp( prenode->mName.C_Str(), original_name.c_str() ) ) {
                        name_carrier = prenode;
                        break;
                    }
                }

                if ( !name_carrier ) {
                    nodes_chain.push_back( new aiNode( original_name ) );
                }

                //setup metadata on newest node
                SetupNodeMetadata( *model, *nodes_chain.back() );

                // link all nodes in a row
                aiNode* last_parent = &parent;
                for( aiNode* prenode : nodes_chain ) {
                    ai_assert( prenode );

                    if ( last_parent != &parent ) {
                        last_parent->mNumChildren = 1;
                        last_parent->mChildren = new aiNode*[ 1 ];
                        last_parent->mChildren[ 0 ] = prenode;
                    }

                    prenode->mParent = last_parent;
                    last_parent = prenode;

                    new_abs_transform *= prenode->mTransformation;
                }

                // attach geometry
                ConvertModel( *model, *nodes_chain.back(), new_abs_transform );

                // attach sub-nodes
                ConvertNodes( model->ID(), *nodes_chain.back(), new_abs_transform );

                if ( doc.Settings().readLights ) {
                    ConvertLights( *model );
                }

                if ( doc.Settings().readCameras ) {
                    ConvertCameras( *model );
                }

                nodes.push_back( nodes_chain.front() );
                nodes_chain.clear();
            }
        }

        if ( nodes.size() ) {
            parent.mChildren = new aiNode*[ nodes.size() ]();
            parent.mNumChildren = static_cast<unsigned int>( nodes.size() );

            std::swap_ranges( nodes.begin(), nodes.end(), parent.mChildren );
        }
    }
    catch ( std::exception& ) {
        Util::delete_fun<aiNode> deleter;
        std::for_each( nodes.begin(), nodes.end(), deleter );
        std::for_each( nodes_chain.begin(), nodes_chain.end(), deleter );
    }
}


void Converter::ConvertLights( const Model& model )
{
    const std::vector<const NodeAttribute*>& node_attrs = model.GetAttributes();
    for( const NodeAttribute* attr : node_attrs ) {
        const Light* const light = dynamic_cast<const Light*>( attr );
        if ( light ) {
            ConvertLight( model, *light );
        }
    }
}

void Converter::ConvertCameras( const Model& model )
{
    const std::vector<const NodeAttribute*>& node_attrs = model.GetAttributes();
    for( const NodeAttribute* attr : node_attrs ) {
        const Camera* const cam = dynamic_cast<const Camera*>( attr );
        if ( cam ) {
            ConvertCamera( model, *cam );
        }
    }
}

void Converter::ConvertLight( const Model& model, const Light& light )
{
    lights.push_back( new aiLight() );
    aiLight* const out_light = lights.back();

    out_light->mName.Set( FixNodeName( model.Name() ) );

    const float intensity = light.Intensity() / 100.0f;
    const aiVector3D& col = light.Color();

    out_light->mColorDiffuse = aiColor3D( col.x, col.y, col.z );
    out_light->mColorDiffuse.r *= intensity;
    out_light->mColorDiffuse.g *= intensity;
    out_light->mColorDiffuse.b *= intensity;

    out_light->mColorSpecular = out_light->mColorDiffuse;

    //lights are defined along negative y direction
    out_light->mPosition = aiVector3D(0.0f);
    out_light->mDirection = aiVector3D(0.0f, -1.0f, 0.0f);
    out_light->mUp = aiVector3D(0.0f, 0.0f, -1.0f);

    switch ( light.LightType() )
    {
    case Light::Type_Point:
        out_light->mType = aiLightSource_POINT;
        break;

    case Light::Type_Directional:
        out_light->mType = aiLightSource_DIRECTIONAL;
        break;

    case Light::Type_Spot:
        out_light->mType = aiLightSource_SPOT;
        out_light->mAngleOuterCone = AI_DEG_TO_RAD( light.OuterAngle() );
        out_light->mAngleInnerCone = AI_DEG_TO_RAD( light.InnerAngle() );
        break;

    case Light::Type_Area:
        FBXImporter::LogWarn( "cannot represent area light, set to UNDEFINED" );
        out_light->mType = aiLightSource_UNDEFINED;
        break;

    case Light::Type_Volume:
        FBXImporter::LogWarn( "cannot represent volume light, set to UNDEFINED" );
        out_light->mType = aiLightSource_UNDEFINED;
        break;
    default:
        ai_assert( false );
    }

    float decay = light.DecayStart();
    switch ( light.DecayType() )
    {
    case Light::Decay_None:
        out_light->mAttenuationConstant = decay;
        out_light->mAttenuationLinear = 0.0f;
        out_light->mAttenuationQuadratic = 0.0f;
        break;
    case Light::Decay_Linear:
        out_light->mAttenuationConstant = 0.0f;
        out_light->mAttenuationLinear = 2.0f / decay;
        out_light->mAttenuationQuadratic = 0.0f;
        break;
    case Light::Decay_Quadratic:
        out_light->mAttenuationConstant = 0.0f;
        out_light->mAttenuationLinear = 0.0f;
        out_light->mAttenuationQuadratic = 2.0f / (decay * decay);
        break;
    case Light::Decay_Cubic:
        FBXImporter::LogWarn( "cannot represent cubic attenuation, set to Quadratic" );
        out_light->mAttenuationQuadratic = 1.0f;
        break;
    default:
        ai_assert( false );
    }
}

void Converter::ConvertCamera( const Model& model, const Camera& cam )
{
    cameras.push_back( new aiCamera() );
    aiCamera* const out_camera = cameras.back();

    out_camera->mName.Set( FixNodeName( model.Name() ) );

    out_camera->mAspect = cam.AspectWidth() / cam.AspectHeight();
    //cameras are defined along positive x direction
    out_camera->mPosition = aiVector3D(0.0f);
    out_camera->mLookAt = aiVector3D(1.0f, 0.0f, 0.0f);
    out_camera->mUp = aiVector3D(0.0f, 1.0f, 0.0f);
    out_camera->mHorizontalFOV = AI_DEG_TO_RAD( cam.FieldOfView() );
    out_camera->mClipPlaneNear = cam.NearPlane();
    out_camera->mClipPlaneFar = cam.FarPlane();
}


const char* Converter::NameTransformationComp( TransformationComp comp )
{
    switch ( comp )
    {
    case TransformationComp_Translation:
        return "Translation";
    case TransformationComp_RotationOffset:
        return "RotationOffset";
    case TransformationComp_RotationPivot:
        return "RotationPivot";
    case TransformationComp_PreRotation:
        return "PreRotation";
    case TransformationComp_Rotation:
        return "Rotation";
    case TransformationComp_PostRotation:
        return "PostRotation";
    case TransformationComp_RotationPivotInverse:
        return "RotationPivotInverse";
    case TransformationComp_ScalingOffset:
        return "ScalingOffset";
    case TransformationComp_ScalingPivot:
        return "ScalingPivot";
    case TransformationComp_Scaling:
        return "Scaling";
    case TransformationComp_ScalingPivotInverse:
        return "ScalingPivotInverse";
    case TransformationComp_GeometricScaling:
        return "GeometricScaling";
    case TransformationComp_GeometricRotation:
        return "GeometricRotation";
    case TransformationComp_GeometricTranslation:
        return "GeometricTranslation";
    case TransformationComp_MAXIMUM: // this is to silence compiler warnings
    default:
        break;
    }

    ai_assert( false );
    return NULL;
}

const char* Converter::NameTransformationCompProperty( TransformationComp comp )
{
    switch ( comp )
    {
    case TransformationComp_Translation:
        return "Lcl Translation";
    case TransformationComp_RotationOffset:
        return "RotationOffset";
    case TransformationComp_RotationPivot:
        return "RotationPivot";
    case TransformationComp_PreRotation:
        return "PreRotation";
    case TransformationComp_Rotation:
        return "Lcl Rotation";
    case TransformationComp_PostRotation:
        return "PostRotation";
    case TransformationComp_RotationPivotInverse:
        return "RotationPivotInverse";
    case TransformationComp_ScalingOffset:
        return "ScalingOffset";
    case TransformationComp_ScalingPivot:
        return "ScalingPivot";
    case TransformationComp_Scaling:
        return "Lcl Scaling";
    case TransformationComp_ScalingPivotInverse:
        return "ScalingPivotInverse";
    case TransformationComp_GeometricScaling:
        return "GeometricScaling";
    case TransformationComp_GeometricRotation:
        return "GeometricRotation";
    case TransformationComp_GeometricTranslation:
        return "GeometricTranslation";
    case TransformationComp_MAXIMUM: // this is to silence compiler warnings
        break;
    }

    ai_assert( false );
    return NULL;
}

aiVector3D Converter::TransformationCompDefaultValue( TransformationComp comp )
{
    // XXX a neat way to solve the never-ending special cases for scaling
    // would be to do everything in log space!
    return comp == TransformationComp_Scaling ? aiVector3D( 1.f, 1.f, 1.f ) : aiVector3D();
}

void Converter::GetRotationMatrix( Model::RotOrder mode, const aiVector3D& rotation, aiMatrix4x4& out )
{
    if ( mode == Model::RotOrder_SphericXYZ ) {
        FBXImporter::LogError( "Unsupported RotationMode: SphericXYZ" );
        out = aiMatrix4x4();
        return;
    }

    const float angle_epsilon = 1e-6f;

    out = aiMatrix4x4();

    bool is_id[ 3 ] = { true, true, true };

    aiMatrix4x4 temp[ 3 ];
    if ( std::fabs( rotation.z ) > angle_epsilon ) {
        aiMatrix4x4::RotationZ( AI_DEG_TO_RAD( rotation.z ), temp[ 2 ] );
        is_id[ 2 ] = false;
    }
    if ( std::fabs( rotation.y ) > angle_epsilon ) {
        aiMatrix4x4::RotationY( AI_DEG_TO_RAD( rotation.y ), temp[ 1 ] );
        is_id[ 1 ] = false;
    }
    if ( std::fabs( rotation.x ) > angle_epsilon ) {
        aiMatrix4x4::RotationX( AI_DEG_TO_RAD( rotation.x ), temp[ 0 ] );
        is_id[ 0 ] = false;
    }

    int order[ 3 ] = { -1, -1, -1 };

    // note: rotation order is inverted since we're left multiplying as is usual in assimp
    switch ( mode )
    {
    case Model::RotOrder_EulerXYZ:
        order[ 0 ] = 2;
        order[ 1 ] = 1;
        order[ 2 ] = 0;
        break;

    case Model::RotOrder_EulerXZY:
        order[ 0 ] = 1;
        order[ 1 ] = 2;
        order[ 2 ] = 0;
        break;

    case Model::RotOrder_EulerYZX:
        order[ 0 ] = 0;
        order[ 1 ] = 2;
        order[ 2 ] = 1;
        break;

    case Model::RotOrder_EulerYXZ:
        order[ 0 ] = 2;
        order[ 1 ] = 0;
        order[ 2 ] = 1;
        break;

    case Model::RotOrder_EulerZXY:
        order[ 0 ] = 1;
        order[ 1 ] = 0;
        order[ 2 ] = 2;
        break;

    case Model::RotOrder_EulerZYX:
        order[ 0 ] = 0;
        order[ 1 ] = 1;
        order[ 2 ] = 2;
        break;

    default:
        ai_assert( false );
    }

    ai_assert( ( order[ 0 ] >= 0 ) && ( order[ 0 ] <= 2 ) );
    ai_assert( ( order[ 1 ] >= 0 ) && ( order[ 1 ] <= 2 ) );
    ai_assert( ( order[ 2 ] >= 0 ) && ( order[ 2 ] <= 2 ) );

    if ( !is_id[ order[ 0 ] ] ) {
        out = temp[ order[ 0 ] ];
    }

    if ( !is_id[ order[ 1 ] ] ) {
        out = out * temp[ order[ 1 ] ];
    }

    if ( !is_id[ order[ 2 ] ] ) {
        out = out * temp[ order[ 2 ] ];
    }
}

bool Converter::NeedsComplexTransformationChain( const Model& model )
{
    const PropertyTable& props = model.Props();
    bool ok;

    const float zero_epsilon = 1e-6f;
    for ( size_t i = 0; i < TransformationComp_MAXIMUM; ++i ) {
        const TransformationComp comp = static_cast< TransformationComp >( i );

        if ( comp == TransformationComp_Rotation || comp == TransformationComp_Scaling || comp == TransformationComp_Translation ||
                comp == TransformationComp_GeometricScaling || comp == TransformationComp_GeometricRotation || comp == TransformationComp_GeometricTranslation ) {
            continue;
        }

        const aiVector3D& v = PropertyGet<aiVector3D>( props, NameTransformationCompProperty( comp ), ok );
        if ( ok && v.SquareLength() > zero_epsilon ) {
            return true;
        }
    }

    return false;
}

std::string Converter::NameTransformationChainNode( const std::string& name, TransformationComp comp )
{
    return name + std::string( MAGIC_NODE_TAG ) + "_" + NameTransformationComp( comp );
}

void Converter::GenerateTransformationNodeChain( const Model& model, std::vector<aiNode*>& output_nodes )
{
    const PropertyTable& props = model.Props();
    const Model::RotOrder rot = model.RotationOrder();

    bool ok;

    aiMatrix4x4 chain[ TransformationComp_MAXIMUM ];
    std::fill_n( chain, static_cast<unsigned int>( TransformationComp_MAXIMUM ), aiMatrix4x4() );

    // generate transformation matrices for all the different transformation components
    const float zero_epsilon = 1e-6f;
    bool is_complex = false;

    const aiVector3D& PreRotation = PropertyGet<aiVector3D>( props, "PreRotation", ok );
    if ( ok && PreRotation.SquareLength() > zero_epsilon ) {
        is_complex = true;

        GetRotationMatrix( rot, PreRotation, chain[ TransformationComp_PreRotation ] );
    }

    const aiVector3D& PostRotation = PropertyGet<aiVector3D>( props, "PostRotation", ok );
    if ( ok && PostRotation.SquareLength() > zero_epsilon ) {
        is_complex = true;

        GetRotationMatrix( rot, PostRotation, chain[ TransformationComp_PostRotation ] );
    }

    const aiVector3D& RotationPivot = PropertyGet<aiVector3D>( props, "RotationPivot", ok );
    if ( ok && RotationPivot.SquareLength() > zero_epsilon ) {
        is_complex = true;

        aiMatrix4x4::Translation( RotationPivot, chain[ TransformationComp_RotationPivot ] );
        aiMatrix4x4::Translation( -RotationPivot, chain[ TransformationComp_RotationPivotInverse ] );
    }

    const aiVector3D& RotationOffset = PropertyGet<aiVector3D>( props, "RotationOffset", ok );
    if ( ok && RotationOffset.SquareLength() > zero_epsilon ) {
        is_complex = true;

        aiMatrix4x4::Translation( RotationOffset, chain[ TransformationComp_RotationOffset ] );
    }

    const aiVector3D& ScalingOffset = PropertyGet<aiVector3D>( props, "ScalingOffset", ok );
    if ( ok && ScalingOffset.SquareLength() > zero_epsilon ) {
        is_complex = true;

        aiMatrix4x4::Translation( ScalingOffset, chain[ TransformationComp_ScalingOffset ] );
    }

    const aiVector3D& ScalingPivot = PropertyGet<aiVector3D>( props, "ScalingPivot", ok );
    if ( ok && ScalingPivot.SquareLength() > zero_epsilon ) {
        is_complex = true;

        aiMatrix4x4::Translation( ScalingPivot, chain[ TransformationComp_ScalingPivot ] );
        aiMatrix4x4::Translation( -ScalingPivot, chain[ TransformationComp_ScalingPivotInverse ] );
    }

    const aiVector3D& Translation = PropertyGet<aiVector3D>( props, "Lcl Translation", ok );
    if ( ok && Translation.SquareLength() > zero_epsilon ) {
        aiMatrix4x4::Translation( Translation, chain[ TransformationComp_Translation ] );
    }

    const aiVector3D& Scaling = PropertyGet<aiVector3D>( props, "Lcl Scaling", ok );
    if ( ok && std::fabs( Scaling.SquareLength() - 1.0f ) > zero_epsilon ) {
        aiMatrix4x4::Scaling( Scaling, chain[ TransformationComp_Scaling ] );
    }

    const aiVector3D& Rotation = PropertyGet<aiVector3D>( props, "Lcl Rotation", ok );
    if ( ok && Rotation.SquareLength() > zero_epsilon ) {
        GetRotationMatrix( rot, Rotation, chain[ TransformationComp_Rotation ] );
    }

    const aiVector3D& GeometricScaling = PropertyGet<aiVector3D>( props, "GeometricScaling", ok );
    if ( ok && std::fabs( GeometricScaling.SquareLength() - 1.0f ) > zero_epsilon ) {
        aiMatrix4x4::Scaling( GeometricScaling, chain[ TransformationComp_GeometricScaling ] );
    }

    const aiVector3D& GeometricRotation = PropertyGet<aiVector3D>( props, "GeometricRotation", ok );
    if ( ok && GeometricRotation.SquareLength() > zero_epsilon ) {
        GetRotationMatrix( rot, GeometricRotation, chain[ TransformationComp_GeometricRotation ] );
    }

    const aiVector3D& GeometricTranslation = PropertyGet<aiVector3D>( props, "GeometricTranslation", ok );
    if ( ok && GeometricTranslation.SquareLength() > zero_epsilon ) {
        aiMatrix4x4::Translation( GeometricTranslation, chain[ TransformationComp_GeometricTranslation ] );
    }

    // is_complex needs to be consistent with NeedsComplexTransformationChain()
    // or the interplay between this code and the animation converter would
    // not be guaranteed.
    ai_assert( NeedsComplexTransformationChain( model ) == is_complex );

    const std::string& name = FixNodeName( model.Name() );

    // now, if we have more than just Translation, Scaling and Rotation,
    // we need to generate a full node chain to accommodate for assimp's
    // lack to express pivots and offsets.
    if ( is_complex && doc.Settings().preservePivots ) {
        FBXImporter::LogInfo( "generating full transformation chain for node: " + name );

        // query the anim_chain_bits dictionary to find out which chain elements
        // have associated node animation channels. These can not be dropped
        // even if they have identity transform in bind pose.
        NodeAnimBitMap::const_iterator it = node_anim_chain_bits.find( name );
        const unsigned int anim_chain_bitmask = ( it == node_anim_chain_bits.end() ? 0 : ( *it ).second );

        unsigned int bit = 0x1;
        for ( size_t i = 0; i < TransformationComp_MAXIMUM; ++i, bit <<= 1 ) {
            const TransformationComp comp = static_cast<TransformationComp>( i );

            if ( chain[ i ].IsIdentity() && ( anim_chain_bitmask & bit ) == 0 ) {
                continue;
            }

            if ( comp == TransformationComp_PostRotation  ) {
                chain[ i ] = chain[ i ].Inverse();
            }

            aiNode* nd = new aiNode();
            output_nodes.push_back( nd );

            nd->mName.Set( NameTransformationChainNode( name, comp ) );
            nd->mTransformation = chain[ i ];
        }

        ai_assert( output_nodes.size() );
        return;
    }

    // else, we can just multiply the matrices together
    aiNode* nd = new aiNode();
    output_nodes.push_back( nd );

    nd->mName.Set( name );

    for (const auto &transform : chain) {
        nd->mTransformation = nd->mTransformation * transform;
    }
}

void Converter::SetupNodeMetadata( const Model& model, aiNode& nd )
{
    const PropertyTable& props = model.Props();
    DirectPropertyMap unparsedProperties = props.GetUnparsedProperties();

    // create metadata on node
    const std::size_t numStaticMetaData = 2;
    aiMetadata* data = aiMetadata::Alloc( static_cast<unsigned int>(unparsedProperties.size() + numStaticMetaData) );
    nd.mMetaData = data;
    int index = 0;

    // find user defined properties (3ds Max)
    data->Set( index++, "UserProperties", aiString( PropertyGet<std::string>( props, "UDP3DSMAX", "" ) ) );
    // preserve the info that a node was marked as Null node in the original file.
    data->Set( index++, "IsNull", model.IsNull() ? true : false );

    // add unparsed properties to the node's metadata
    for( const DirectPropertyMap::value_type& prop : unparsedProperties ) {
        // Interpret the property as a concrete type
        if ( const TypedProperty<bool>* interpreted = prop.second->As<TypedProperty<bool> >() ) {
            data->Set( index++, prop.first, interpreted->Value() );
        } else if ( const TypedProperty<int>* interpreted = prop.second->As<TypedProperty<int> >() ) {
            data->Set( index++, prop.first, interpreted->Value() );
        } else if ( const TypedProperty<uint64_t>* interpreted = prop.second->As<TypedProperty<uint64_t> >() ) {
            data->Set( index++, prop.first, interpreted->Value() );
        } else if ( const TypedProperty<float>* interpreted = prop.second->As<TypedProperty<float> >() ) {
            data->Set( index++, prop.first, interpreted->Value() );
        } else if ( const TypedProperty<std::string>* interpreted = prop.second->As<TypedProperty<std::string> >() ) {
            data->Set( index++, prop.first, aiString( interpreted->Value() ) );
        } else if ( const TypedProperty<aiVector3D>* interpreted = prop.second->As<TypedProperty<aiVector3D> >() ) {
            data->Set( index++, prop.first, interpreted->Value() );
        } else {
            ai_assert( false );
        }
    }
}

void Converter::ConvertModel( const Model& model, aiNode& nd, const aiMatrix4x4& node_global_transform )
{
    const std::vector<const Geometry*>& geos = model.GetGeometry();

    std::vector<unsigned int> meshes;
    meshes.reserve( geos.size() );

    for( const Geometry* geo : geos ) {

        const MeshGeometry* const mesh = dynamic_cast< const MeshGeometry* >( geo );
        if ( mesh ) {
            const std::vector<unsigned int>& indices = ConvertMesh( *mesh, model, node_global_transform );
            std::copy( indices.begin(), indices.end(), std::back_inserter( meshes ) );
        }
        else {
            FBXImporter::LogWarn( "ignoring unrecognized geometry: " + geo->Name() );
        }
    }

    if ( meshes.size() ) {
        nd.mMeshes = new unsigned int[ meshes.size() ]();
        nd.mNumMeshes = static_cast< unsigned int >( meshes.size() );

        std::swap_ranges( meshes.begin(), meshes.end(), nd.mMeshes );
    }
}

std::vector<unsigned int> Converter::ConvertMesh( const MeshGeometry& mesh, const Model& model,
    const aiMatrix4x4& node_global_transform )
{
    std::vector<unsigned int> temp;

    MeshMap::const_iterator it = meshes_converted.find( &mesh );
    if ( it != meshes_converted.end() ) {
        std::copy( ( *it ).second.begin(), ( *it ).second.end(), std::back_inserter( temp ) );
        return temp;
    }

    const std::vector<aiVector3D>& vertices = mesh.GetVertices();
    const std::vector<unsigned int>& faces = mesh.GetFaceIndexCounts();
    if ( vertices.empty() || faces.empty() ) {
        FBXImporter::LogWarn( "ignoring empty geometry: " + mesh.Name() );
        return temp;
    }

    // one material per mesh maps easily to aiMesh. Multiple material
    // meshes need to be split.
    const MatIndexArray& mindices = mesh.GetMaterialIndices();
    if ( doc.Settings().readMaterials && !mindices.empty() ) {
        const MatIndexArray::value_type base = mindices[ 0 ];
        for( MatIndexArray::value_type index : mindices ) {
            if ( index != base ) {
                return ConvertMeshMultiMaterial( mesh, model, node_global_transform );
            }
        }
    }

    // faster code-path, just copy the data
    temp.push_back( ConvertMeshSingleMaterial( mesh, model, node_global_transform ) );
    return temp;
}

aiMesh* Converter::SetupEmptyMesh( const MeshGeometry& mesh )
{
    aiMesh* const out_mesh = new aiMesh();
    meshes.push_back( out_mesh );
    meshes_converted[ &mesh ].push_back( static_cast<unsigned int>( meshes.size() - 1 ) );

    // set name
    std::string name = mesh.Name();
    if ( name.substr( 0, 10 ) == "Geometry::" ) {
        name = name.substr( 10 );
    }

    if ( name.length() ) {
        out_mesh->mName.Set( name );
    }

    return out_mesh;
}

unsigned int Converter::ConvertMeshSingleMaterial( const MeshGeometry& mesh, const Model& model,
    const aiMatrix4x4& node_global_transform )
{
    const MatIndexArray& mindices = mesh.GetMaterialIndices();
    aiMesh* const out_mesh = SetupEmptyMesh( mesh );

    const std::vector<aiVector3D>& vertices = mesh.GetVertices();
    const std::vector<unsigned int>& faces = mesh.GetFaceIndexCounts();

    // copy vertices
    out_mesh->mNumVertices = static_cast<unsigned int>( vertices.size() );
    out_mesh->mVertices = new aiVector3D[ vertices.size() ];
    std::copy( vertices.begin(), vertices.end(), out_mesh->mVertices );

    // generate dummy faces
    out_mesh->mNumFaces = static_cast<unsigned int>( faces.size() );
    aiFace* fac = out_mesh->mFaces = new aiFace[ faces.size() ]();

    unsigned int cursor = 0;
    for( unsigned int pcount : faces ) {
        aiFace& f = *fac++;
        f.mNumIndices = pcount;
        f.mIndices = new unsigned int[ pcount ];
        switch ( pcount )
        {
        case 1:
            out_mesh->mPrimitiveTypes |= aiPrimitiveType_POINT;
            break;
        case 2:
            out_mesh->mPrimitiveTypes |= aiPrimitiveType_LINE;
            break;
        case 3:
            out_mesh->mPrimitiveTypes |= aiPrimitiveType_TRIANGLE;
            break;
        default:
            out_mesh->mPrimitiveTypes |= aiPrimitiveType_POLYGON;
            break;
        }
        for ( unsigned int i = 0; i < pcount; ++i ) {
            f.mIndices[ i ] = cursor++;
        }
    }

    // copy normals
    const std::vector<aiVector3D>& normals = mesh.GetNormals();
    if ( normals.size() ) {
        ai_assert( normals.size() == vertices.size() );

        out_mesh->mNormals = new aiVector3D[ vertices.size() ];
        std::copy( normals.begin(), normals.end(), out_mesh->mNormals );
    }

    // copy tangents - assimp requires both tangents and bitangents (binormals)
    // to be present, or neither of them. Compute binormals from normals
    // and tangents if needed.
    const std::vector<aiVector3D>& tangents = mesh.GetTangents();
    const std::vector<aiVector3D>* binormals = &mesh.GetBinormals();

    if ( tangents.size() ) {
        std::vector<aiVector3D> tempBinormals;
        if ( !binormals->size() ) {
            if ( normals.size() ) {
                tempBinormals.resize( normals.size() );
                for ( unsigned int i = 0; i < tangents.size(); ++i ) {
                    tempBinormals[ i ] = normals[ i ] ^ tangents[ i ];
                }

                binormals = &tempBinormals;
            }
            else {
                binormals = NULL;
            }
        }

        if ( binormals ) {
            ai_assert( tangents.size() == vertices.size() );
            ai_assert( binormals->size() == vertices.size() );

            out_mesh->mTangents = new aiVector3D[ vertices.size() ];
            std::copy( tangents.begin(), tangents.end(), out_mesh->mTangents );

            out_mesh->mBitangents = new aiVector3D[ vertices.size() ];
            std::copy( binormals->begin(), binormals->end(), out_mesh->mBitangents );
        }
    }

    // copy texture coords
    for ( unsigned int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i ) {
        const std::vector<aiVector2D>& uvs = mesh.GetTextureCoords( i );
        if ( uvs.empty() ) {
            break;
        }

        aiVector3D* out_uv = out_mesh->mTextureCoords[ i ] = new aiVector3D[ vertices.size() ];
        for( const aiVector2D& v : uvs ) {
            *out_uv++ = aiVector3D( v.x, v.y, 0.0f );
        }

        out_mesh->mNumUVComponents[ i ] = 2;
    }

    // copy vertex colors
    for ( unsigned int i = 0; i < AI_MAX_NUMBER_OF_COLOR_SETS; ++i ) {
        const std::vector<aiColor4D>& colors = mesh.GetVertexColors( i );
        if ( colors.empty() ) {
            break;
        }

        out_mesh->mColors[ i ] = new aiColor4D[ vertices.size() ];
        std::copy( colors.begin(), colors.end(), out_mesh->mColors[ i ] );
    }

    if ( !doc.Settings().readMaterials || mindices.empty() ) {
        FBXImporter::LogError( "no material assigned to mesh, setting default material" );
        out_mesh->mMaterialIndex = GetDefaultMaterial();
    }
    else {
        ConvertMaterialForMesh( out_mesh, model, mesh, mindices[ 0 ] );
    }

    if ( doc.Settings().readWeights && mesh.DeformerSkin() != NULL ) {
        ConvertWeights( out_mesh, model, mesh, node_global_transform, NO_MATERIAL_SEPARATION );
    }

    return static_cast<unsigned int>( meshes.size() - 1 );
}

std::vector<unsigned int> Converter::ConvertMeshMultiMaterial( const MeshGeometry& mesh, const Model& model,
    const aiMatrix4x4& node_global_transform )
{
    const MatIndexArray& mindices = mesh.GetMaterialIndices();
    ai_assert( mindices.size() );

    std::set<MatIndexArray::value_type> had;
    std::vector<unsigned int> indices;

    for( MatIndexArray::value_type index : mindices ) {
        if ( had.find( index ) == had.end() ) {

            indices.push_back( ConvertMeshMultiMaterial( mesh, model, index, node_global_transform ) );
            had.insert( index );
        }
    }

    return indices;
}

unsigned int Converter::ConvertMeshMultiMaterial( const MeshGeometry& mesh, const Model& model,
    MatIndexArray::value_type index,
    const aiMatrix4x4& node_global_transform )
{
    aiMesh* const out_mesh = SetupEmptyMesh( mesh );

    const MatIndexArray& mindices = mesh.GetMaterialIndices();
    const std::vector<aiVector3D>& vertices = mesh.GetVertices();
    const std::vector<unsigned int>& faces = mesh.GetFaceIndexCounts();

    const bool process_weights = doc.Settings().readWeights && mesh.DeformerSkin() != NULL;

    unsigned int count_faces = 0;
    unsigned int count_vertices = 0;

    // count faces
    std::vector<unsigned int>::const_iterator itf = faces.begin();
    for ( MatIndexArray::const_iterator it = mindices.begin(),
        end = mindices.end(); it != end; ++it, ++itf )
    {
        if ( ( *it ) != index ) {
            continue;
        }
        ++count_faces;
        count_vertices += *itf;
    }

    ai_assert( count_faces );
    ai_assert( count_vertices );

    // mapping from output indices to DOM indexing, needed to resolve weights
    std::vector<unsigned int> reverseMapping;

    if ( process_weights ) {
        reverseMapping.resize( count_vertices );
    }

    // allocate output data arrays, but don't fill them yet
    out_mesh->mNumVertices = count_vertices;
    out_mesh->mVertices = new aiVector3D[ count_vertices ];

    out_mesh->mNumFaces = count_faces;
    aiFace* fac = out_mesh->mFaces = new aiFace[ count_faces ]();


    // allocate normals
    const std::vector<aiVector3D>& normals = mesh.GetNormals();
    if ( normals.size() ) {
        ai_assert( normals.size() == vertices.size() );
        out_mesh->mNormals = new aiVector3D[ vertices.size() ];
    }

    // allocate tangents, binormals.
    const std::vector<aiVector3D>& tangents = mesh.GetTangents();
    const std::vector<aiVector3D>* binormals = &mesh.GetBinormals();
    std::vector<aiVector3D> tempBinormals;

    if ( tangents.size() ) {
        if ( !binormals->size() ) {
            if ( normals.size() ) {
                // XXX this computes the binormals for the entire mesh, not only
                // the part for which we need them.
                tempBinormals.resize( normals.size() );
                for ( unsigned int i = 0; i < tangents.size(); ++i ) {
                    tempBinormals[ i ] = normals[ i ] ^ tangents[ i ];
                }

                binormals = &tempBinormals;
            }
            else {
                binormals = NULL;
            }
        }

        if ( binormals ) {
            ai_assert( tangents.size() == vertices.size() && binormals->size() == vertices.size() );

            out_mesh->mTangents = new aiVector3D[ vertices.size() ];
            out_mesh->mBitangents = new aiVector3D[ vertices.size() ];
        }
    }

    // allocate texture coords
    unsigned int num_uvs = 0;
    for ( unsigned int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i, ++num_uvs ) {
        const std::vector<aiVector2D>& uvs = mesh.GetTextureCoords( i );
        if ( uvs.empty() ) {
            break;
        }

        out_mesh->mTextureCoords[ i ] = new aiVector3D[ vertices.size() ];
        out_mesh->mNumUVComponents[ i ] = 2;
    }

    // allocate vertex colors
    unsigned int num_vcs = 0;
    for ( unsigned int i = 0; i < AI_MAX_NUMBER_OF_COLOR_SETS; ++i, ++num_vcs ) {
        const std::vector<aiColor4D>& colors = mesh.GetVertexColors( i );
        if ( colors.empty() ) {
            break;
        }

        out_mesh->mColors[ i ] = new aiColor4D[ vertices.size() ];
    }

    unsigned int cursor = 0, in_cursor = 0;

    itf = faces.begin();
    for ( MatIndexArray::const_iterator it = mindices.begin(),
        end = mindices.end(); it != end; ++it, ++itf )
    {
        const unsigned int pcount = *itf;
        if ( ( *it ) != index ) {
            in_cursor += pcount;
            continue;
        }

        aiFace& f = *fac++;

        f.mNumIndices = pcount;
        f.mIndices = new unsigned int[ pcount ];
        switch ( pcount )
        {
        case 1:
            out_mesh->mPrimitiveTypes |= aiPrimitiveType_POINT;
            break;
        case 2:
            out_mesh->mPrimitiveTypes |= aiPrimitiveType_LINE;
            break;
        case 3:
            out_mesh->mPrimitiveTypes |= aiPrimitiveType_TRIANGLE;
            break;
        default:
            out_mesh->mPrimitiveTypes |= aiPrimitiveType_POLYGON;
            break;
        }
        for ( unsigned int i = 0; i < pcount; ++i, ++cursor, ++in_cursor ) {
            f.mIndices[ i ] = cursor;

            if ( reverseMapping.size() ) {
                reverseMapping[ cursor ] = in_cursor;
            }

            out_mesh->mVertices[ cursor ] = vertices[ in_cursor ];

            if ( out_mesh->mNormals ) {
                out_mesh->mNormals[ cursor ] = normals[ in_cursor ];
            }

            if ( out_mesh->mTangents ) {
                out_mesh->mTangents[ cursor ] = tangents[ in_cursor ];
                out_mesh->mBitangents[ cursor ] = ( *binormals )[ in_cursor ];
            }

            for ( unsigned int i = 0; i < num_uvs; ++i ) {
                const std::vector<aiVector2D>& uvs = mesh.GetTextureCoords( i );
                out_mesh->mTextureCoords[ i ][ cursor ] = aiVector3D( uvs[ in_cursor ].x, uvs[ in_cursor ].y, 0.0f );
            }

            for ( unsigned int i = 0; i < num_vcs; ++i ) {
                const std::vector<aiColor4D>& cols = mesh.GetVertexColors( i );
                out_mesh->mColors[ i ][ cursor ] = cols[ in_cursor ];
            }
        }
    }

    ConvertMaterialForMesh( out_mesh, model, mesh, index );

    if ( process_weights ) {
        ConvertWeights( out_mesh, model, mesh, node_global_transform, index, &reverseMapping );
    }

    return static_cast<unsigned int>( meshes.size() - 1 );
}

void Converter::ConvertWeights( aiMesh* out, const Model& model, const MeshGeometry& geo,
    const aiMatrix4x4& node_global_transform ,
    unsigned int materialIndex,
    std::vector<unsigned int>* outputVertStartIndices  )
{
    ai_assert( geo.DeformerSkin() );

    std::vector<size_t> out_indices;
    std::vector<size_t> index_out_indices;
    std::vector<size_t> count_out_indices;

    const Skin& sk = *geo.DeformerSkin();

    std::vector<aiBone*> bones;
    bones.reserve( sk.Clusters().size() );

    const bool no_mat_check = materialIndex == NO_MATERIAL_SEPARATION;
    ai_assert( no_mat_check || outputVertStartIndices );

    try {

        for( const Cluster* cluster : sk.Clusters() ) {
            ai_assert( cluster );

            const WeightIndexArray& indices = cluster->GetIndices();

            if ( indices.empty() ) {
                continue;
            }

            const MatIndexArray& mats = geo.GetMaterialIndices();

            bool ok = false;

            const size_t no_index_sentinel = std::numeric_limits<size_t>::max();

            count_out_indices.clear();
            index_out_indices.clear();
            out_indices.clear();

            // now check if *any* of these weights is contained in the output mesh,
            // taking notes so we don't need to do it twice.
            for( WeightIndexArray::value_type index : indices ) {

                unsigned int count = 0;
                const unsigned int* const out_idx = geo.ToOutputVertexIndex( index, count );
                // ToOutputVertexIndex only returns NULL if index is out of bounds
                // which should never happen
                ai_assert( out_idx != NULL );

                index_out_indices.push_back( no_index_sentinel );
                count_out_indices.push_back( 0 );

                for ( unsigned int i = 0; i < count; ++i ) {
                    if ( no_mat_check || static_cast<size_t>( mats[ geo.FaceForVertexIndex( out_idx[ i ] ) ] ) == materialIndex ) {

                        if ( index_out_indices.back() == no_index_sentinel ) {
                            index_out_indices.back() = out_indices.size();

                        }

                        if ( no_mat_check ) {
                            out_indices.push_back( out_idx[ i ] );
                        }
                        else {
                            // this extra lookup is in O(logn), so the entire algorithm becomes O(nlogn)
                            const std::vector<unsigned int>::iterator it = std::lower_bound(
                                outputVertStartIndices->begin(),
                                outputVertStartIndices->end(),
                                out_idx[ i ]
                                );

                            out_indices.push_back( std::distance( outputVertStartIndices->begin(), it ) );
                        }

                        ++count_out_indices.back();
                        ok = true;
                    }
                }
            }

            // if we found at least one, generate the output bones
            // XXX this could be heavily simplified by collecting the bone
            // data in a single step.
            if ( ok ) {
                ConvertCluster( bones, model, *cluster, out_indices, index_out_indices,
                    count_out_indices, node_global_transform );
            }
        }
    }
    catch ( std::exception& ) {
        std::for_each( bones.begin(), bones.end(), Util::delete_fun<aiBone>() );
        throw;
    }

    if ( bones.empty() ) {
        return;
    }

    out->mBones = new aiBone*[ bones.size() ]();
    out->mNumBones = static_cast<unsigned int>( bones.size() );

    std::swap_ranges( bones.begin(), bones.end(), out->mBones );
}

void Converter::ConvertCluster( std::vector<aiBone*>& bones, const Model& /*model*/, const Cluster& cl,
        std::vector<size_t>& out_indices,
        std::vector<size_t>& index_out_indices,
        std::vector<size_t>& count_out_indices,
        const aiMatrix4x4& node_global_transform )
{

    aiBone* const bone = new aiBone();
    bones.push_back( bone );

    bone->mName = FixNodeName( cl.TargetNode()->Name() );

    bone->mOffsetMatrix = cl.TransformLink();
    bone->mOffsetMatrix.Inverse();

    bone->mOffsetMatrix = bone->mOffsetMatrix * node_global_transform;

    bone->mNumWeights = static_cast<unsigned int>( out_indices.size() );
    aiVertexWeight* cursor = bone->mWeights = new aiVertexWeight[ out_indices.size() ];

    const size_t no_index_sentinel = std::numeric_limits<size_t>::max();
    const WeightArray& weights = cl.GetWeights();

    const size_t c = index_out_indices.size();
    for ( size_t i = 0; i < c; ++i ) {
        const size_t index_index = index_out_indices[ i ];

        if ( index_index == no_index_sentinel ) {
            continue;
        }

        const size_t cc = count_out_indices[ i ];
        for ( size_t j = 0; j < cc; ++j ) {
            aiVertexWeight& out_weight = *cursor++;

            out_weight.mVertexId = static_cast<unsigned int>( out_indices[ index_index + j ] );
            out_weight.mWeight = weights[ i ];
        }
    }
}

void Converter::ConvertMaterialForMesh( aiMesh* out, const Model& model, const MeshGeometry& geo,
    MatIndexArray::value_type materialIndex )
{
    // locate source materials for this mesh
    const std::vector<const Material*>& mats = model.GetMaterials();
    if ( static_cast<unsigned int>( materialIndex ) >= mats.size() || materialIndex < 0 ) {
        FBXImporter::LogError( "material index out of bounds, setting default material" );
        out->mMaterialIndex = GetDefaultMaterial();
        return;
    }

    const Material* const mat = mats[ materialIndex ];
    MaterialMap::const_iterator it = materials_converted.find( mat );
    if ( it != materials_converted.end() ) {
        out->mMaterialIndex = ( *it ).second;
        return;
    }

    out->mMaterialIndex = ConvertMaterial( *mat, &geo );
    materials_converted[ mat ] = out->mMaterialIndex;
}

unsigned int Converter::GetDefaultMaterial()
{
    if ( defaultMaterialIndex ) {
        return defaultMaterialIndex - 1;
    }

    aiMaterial* out_mat = new aiMaterial();
    materials.push_back( out_mat );

    const aiColor3D diffuse = aiColor3D( 0.8f, 0.8f, 0.8f );
    out_mat->AddProperty( &diffuse, 1, AI_MATKEY_COLOR_DIFFUSE );

    aiString s;
    s.Set( AI_DEFAULT_MATERIAL_NAME );

    out_mat->AddProperty( &s, AI_MATKEY_NAME );

    defaultMaterialIndex = static_cast< unsigned int >( materials.size() );
    return defaultMaterialIndex - 1;
}


unsigned int Converter::ConvertMaterial( const Material& material, const MeshGeometry* const mesh )
{
    const PropertyTable& props = material.Props();

    // generate empty output material
    aiMaterial* out_mat = new aiMaterial();
    materials_converted[ &material ] = static_cast<unsigned int>( materials.size() );

    materials.push_back( out_mat );

    aiString str;

    // stip Material:: prefix
    std::string name = material.Name();
    if ( name.substr( 0, 10 ) == "Material::" ) {
        name = name.substr( 10 );
    }

    // set material name if not empty - this could happen
    // and there should be no key for it in this case.
    if ( name.length() ) {
        str.Set( name );
        out_mat->AddProperty( &str, AI_MATKEY_NAME );
    }

    // shading stuff and colors
    SetShadingPropertiesCommon( out_mat, props );

    // texture assignments
    SetTextureProperties( out_mat, material.Textures(), mesh );
    SetTextureProperties( out_mat, material.LayeredTextures(), mesh );

    return static_cast<unsigned int>( materials.size() - 1 );
}

unsigned int Converter::ConvertVideo( const Video& video )
{
    // generate empty output texture
    aiTexture* out_tex = new aiTexture();
    textures.push_back( out_tex );

    // assuming the texture is compressed
    out_tex->mWidth = static_cast<unsigned int>( video.ContentLength() ); // total data size
    out_tex->mHeight = 0; // fixed to 0

    // steal the data from the Video to avoid an additional copy
    out_tex->pcData = reinterpret_cast<aiTexel*>( const_cast<Video&>( video ).RelinquishContent() );

    // try to extract a hint from the file extension
    const std::string& filename = video.FileName().empty() ? video.RelativeFilename() : video.FileName();
    std::string ext = BaseImporter::GetExtension( filename );

    if ( ext == "jpeg" ) {
        ext = "jpg";
    }

    if ( ext.size() <= 3 ) {
        memcpy( out_tex->achFormatHint, ext.c_str(), ext.size() );
    }

    return static_cast<unsigned int>( textures.size() - 1 );
}

void Converter::TrySetTextureProperties( aiMaterial* out_mat, const TextureMap& textures,
    const std::string& propName,
    aiTextureType target, const MeshGeometry* const mesh )
{
    TextureMap::const_iterator it = textures.find( propName );
    if ( it == textures.end() ) {
        return;
    }

    const Texture* const tex = ( *it ).second;
    if ( tex != 0 )
    {
        aiString path;
        path.Set( tex->RelativeFilename() );

        const Video* media = tex->Media();
        if (media != 0) {
			bool textureReady = false; //tells if our texture is ready (if it was loaded or if it was found)
			unsigned int index;

			VideoMap::const_iterator it = textures_converted.find(media);
			if (it != textures_converted.end()) {
				index = (*it).second;
				textureReady = true;
			}
			else {
				if (media->ContentLength() > 0) {
					index = ConvertVideo(*media);
					textures_converted[media] = index;
					textureReady = true;
				}
				else if (doc.Settings().searchEmbeddedTextures) { //try to find the texture on the already-loaded textures by the filename, if the flag is on					
					textureReady = FindTextureIndexByFilename(*media, index);
				}
			}

			// setup texture reference string (copied from ColladaLoader::FindFilenameForEffectTexture), if the texture is ready
			if (textureReady) {
				path.data[0] = '*';
				path.length = 1 + ASSIMP_itoa10(path.data + 1, MAXLEN - 1, index);
			}
		}  

        out_mat->AddProperty( &path, _AI_MATKEY_TEXTURE_BASE, target, 0 );

        aiUVTransform uvTrafo;
        // XXX handle all kinds of UV transformations
        uvTrafo.mScaling = tex->UVScaling();
        uvTrafo.mTranslation = tex->UVTranslation();
        out_mat->AddProperty( &uvTrafo, 1, _AI_MATKEY_UVTRANSFORM_BASE, target, 0 );

        const PropertyTable& props = tex->Props();

        int uvIndex = 0;

        bool ok;
        const std::string& uvSet = PropertyGet<std::string>( props, "UVSet", ok );
        if ( ok ) {
            // "default" is the name which usually appears in the FbxFileTexture template
            if ( uvSet != "default" && uvSet.length() ) {
                // this is a bit awkward - we need to find a mesh that uses this
                // material and scan its UV channels for the given UV name because
                // assimp references UV channels by index, not by name.

                // XXX: the case that UV channels may appear in different orders
                // in meshes is unhandled. A possible solution would be to sort
                // the UV channels alphabetically, but this would have the side
                // effect that the primary (first) UV channel would sometimes
                // be moved, causing trouble when users read only the first
                // UV channel and ignore UV channel assignments altogether.

                const unsigned int matIndex = static_cast<unsigned int>( std::distance( materials.begin(),
                    std::find( materials.begin(), materials.end(), out_mat )
                    ) );


                uvIndex = -1;
                if ( !mesh )
                {
                    for( const MeshMap::value_type& v : meshes_converted ) {
                        const MeshGeometry* const mesh = dynamic_cast<const MeshGeometry*> ( v.first );
                        if ( !mesh ) {
                            continue;
                        }

                        const MatIndexArray& mats = mesh->GetMaterialIndices();
                        if ( std::find( mats.begin(), mats.end(), matIndex ) == mats.end() ) {
                            continue;
                        }

                        int index = -1;
                        for ( unsigned int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i ) {
                            if ( mesh->GetTextureCoords( i ).empty() ) {
                                break;
                            }
                            const std::string& name = mesh->GetTextureCoordChannelName( i );
                            if ( name == uvSet ) {
                                index = static_cast<int>( i );
                                break;
                            }
                        }
                        if ( index == -1 ) {
                            FBXImporter::LogWarn( "did not find UV channel named " + uvSet + " in a mesh using this material" );
                            continue;
                        }

                        if ( uvIndex == -1 ) {
                            uvIndex = index;
                        }
                        else {
                            FBXImporter::LogWarn( "the UV channel named " + uvSet +
                                " appears at different positions in meshes, results will be wrong" );
                        }
                    }
                }
                else
                {
                    int index = -1;
                    for ( unsigned int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i ) {
                        if ( mesh->GetTextureCoords( i ).empty() ) {
                            break;
                        }
                        const std::string& name = mesh->GetTextureCoordChannelName( i );
                        if ( name == uvSet ) {
                            index = static_cast<int>( i );
                            break;
                        }
                    }
                    if ( index == -1 ) {
                        FBXImporter::LogWarn( "did not find UV channel named " + uvSet + " in a mesh using this material" );
                    }

                    if ( uvIndex == -1 ) {
                        uvIndex = index;
                    }
                }

                if ( uvIndex == -1 ) {
                    FBXImporter::LogWarn( "failed to resolve UV channel " + uvSet + ", using first UV channel" );
                    uvIndex = 0;
                }
            }
        }

        out_mat->AddProperty( &uvIndex, 1, _AI_MATKEY_UVWSRC_BASE, target, 0 );
    }
}

void Converter::TrySetTextureProperties( aiMaterial* out_mat, const LayeredTextureMap& layeredTextures,
    const std::string& propName,
    aiTextureType target, const MeshGeometry* const mesh )
{
    LayeredTextureMap::const_iterator it = layeredTextures.find( propName );
    if ( it == layeredTextures.end() ) {
        return;
    }

    int texCount = (*it).second->textureCount();
    
    // Set the blend mode for layered textures
	int blendmode= (*it).second->GetBlendMode();
	out_mat->AddProperty(&blendmode,1,_AI_MATKEY_TEXOP_BASE,target,0);

	for(int texIndex = 0; texIndex < texCount; texIndex++){
    
        const Texture* const tex = ( *it ).second->getTexture(texIndex);

        aiString path;
        path.Set( tex->RelativeFilename() );

        out_mat->AddProperty( &path, _AI_MATKEY_TEXTURE_BASE, target, texIndex );

        aiUVTransform uvTrafo;
        // XXX handle all kinds of UV transformations
        uvTrafo.mScaling = tex->UVScaling();
        uvTrafo.mTranslation = tex->UVTranslation();
        out_mat->AddProperty( &uvTrafo, 1, _AI_MATKEY_UVTRANSFORM_BASE, target, texIndex );

        const PropertyTable& props = tex->Props();

        int uvIndex = 0;

        bool ok;
        const std::string& uvSet = PropertyGet<std::string>( props, "UVSet", ok );
        if ( ok ) {
            // "default" is the name which usually appears in the FbxFileTexture template
            if ( uvSet != "default" && uvSet.length() ) {
                // this is a bit awkward - we need to find a mesh that uses this
                // material and scan its UV channels for the given UV name because
                // assimp references UV channels by index, not by name.

                // XXX: the case that UV channels may appear in different orders
                // in meshes is unhandled. A possible solution would be to sort
                // the UV channels alphabetically, but this would have the side
                // effect that the primary (first) UV channel would sometimes
                // be moved, causing trouble when users read only the first
                // UV channel and ignore UV channel assignments altogether.

                const unsigned int matIndex = static_cast<unsigned int>( std::distance( materials.begin(),
                    std::find( materials.begin(), materials.end(), out_mat )
                    ) );

                uvIndex = -1;
                if ( !mesh )
                {
                    for( const MeshMap::value_type& v : meshes_converted ) {
                        const MeshGeometry* const mesh = dynamic_cast<const MeshGeometry*> ( v.first );
                        if ( !mesh ) {
                            continue;
                        }

                        const MatIndexArray& mats = mesh->GetMaterialIndices();
                        if ( std::find( mats.begin(), mats.end(), matIndex ) == mats.end() ) {
                            continue;
                        }

                        int index = -1;
                        for ( unsigned int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i ) {
                            if ( mesh->GetTextureCoords( i ).empty() ) {
                                break;
                            }
                            const std::string& name = mesh->GetTextureCoordChannelName( i );
                            if ( name == uvSet ) {
                                index = static_cast<int>( i );
                                break;
                            }
                        }
                        if ( index == -1 ) {
                            FBXImporter::LogWarn( "did not find UV channel named " + uvSet + " in a mesh using this material" );
                            continue;
                        }

                        if ( uvIndex == -1 ) {
                            uvIndex = index;
                        }
                        else {
                            FBXImporter::LogWarn( "the UV channel named " + uvSet +
                                " appears at different positions in meshes, results will be wrong" );
                        }
                    }
                }
                else
                {
                    int index = -1;
                    for ( unsigned int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i ) {
                        if ( mesh->GetTextureCoords( i ).empty() ) {
                            break;
                        }
                        const std::string& name = mesh->GetTextureCoordChannelName( i );
                        if ( name == uvSet ) {
                            index = static_cast<int>( i );
                            break;
                        }
                    }
                    if ( index == -1 ) {
                        FBXImporter::LogWarn( "did not find UV channel named " + uvSet + " in a mesh using this material" );
                    }

                    if ( uvIndex == -1 ) {
                        uvIndex = index;
                    }
                }

                if ( uvIndex == -1 ) {
                    FBXImporter::LogWarn( "failed to resolve UV channel " + uvSet + ", using first UV channel" );
                    uvIndex = 0;
                }
            }
        }

        out_mat->AddProperty( &uvIndex, 1, _AI_MATKEY_UVWSRC_BASE, target, texIndex );
    }
}

void Converter::SetTextureProperties( aiMaterial* out_mat, const TextureMap& textures, const MeshGeometry* const mesh )
{
    TrySetTextureProperties( out_mat, textures, "DiffuseColor", aiTextureType_DIFFUSE, mesh );
    TrySetTextureProperties( out_mat, textures, "AmbientColor", aiTextureType_AMBIENT, mesh );
    TrySetTextureProperties( out_mat, textures, "EmissiveColor", aiTextureType_EMISSIVE, mesh );
    TrySetTextureProperties( out_mat, textures, "SpecularColor", aiTextureType_SPECULAR, mesh );
    TrySetTextureProperties( out_mat, textures, "SpecularFactor", aiTextureType_SPECULAR, mesh);
    TrySetTextureProperties( out_mat, textures, "TransparentColor", aiTextureType_OPACITY, mesh );
    TrySetTextureProperties( out_mat, textures, "ReflectionColor", aiTextureType_REFLECTION, mesh );
    TrySetTextureProperties( out_mat, textures, "DisplacementColor", aiTextureType_DISPLACEMENT, mesh );
    TrySetTextureProperties( out_mat, textures, "NormalMap", aiTextureType_NORMALS, mesh );
    TrySetTextureProperties( out_mat, textures, "Bump", aiTextureType_HEIGHT, mesh );
    TrySetTextureProperties( out_mat, textures, "ShininessExponent", aiTextureType_SHININESS, mesh );
}

void Converter::SetTextureProperties( aiMaterial* out_mat, const LayeredTextureMap& layeredTextures, const MeshGeometry* const mesh )
{
    TrySetTextureProperties( out_mat, layeredTextures, "DiffuseColor", aiTextureType_DIFFUSE, mesh );
    TrySetTextureProperties( out_mat, layeredTextures, "AmbientColor", aiTextureType_AMBIENT, mesh );
    TrySetTextureProperties( out_mat, layeredTextures, "EmissiveColor", aiTextureType_EMISSIVE, mesh );
    TrySetTextureProperties( out_mat, layeredTextures, "SpecularColor", aiTextureType_SPECULAR, mesh );
    TrySetTextureProperties( out_mat, layeredTextures, "SpecularFactor", aiTextureType_SPECULAR, mesh);
    TrySetTextureProperties( out_mat, layeredTextures, "TransparentColor", aiTextureType_OPACITY, mesh );
    TrySetTextureProperties( out_mat, layeredTextures, "ReflectionColor", aiTextureType_REFLECTION, mesh );
    TrySetTextureProperties( out_mat, layeredTextures, "DisplacementColor", aiTextureType_DISPLACEMENT, mesh );
    TrySetTextureProperties( out_mat, layeredTextures, "NormalMap", aiTextureType_NORMALS, mesh );
    TrySetTextureProperties( out_mat, layeredTextures, "Bump", aiTextureType_HEIGHT, mesh );
    TrySetTextureProperties( out_mat, layeredTextures, "ShininessExponent", aiTextureType_SHININESS, mesh );
}

aiColor3D Converter::GetColorPropertyFromMaterial( const PropertyTable& props, const std::string& baseName,
    bool& result )
{
    result = true;

    bool ok;
    const aiVector3D& Diffuse = PropertyGet<aiVector3D>( props, baseName, ok );
    if ( ok ) {
        return aiColor3D( Diffuse.x, Diffuse.y, Diffuse.z );
    }
    else {
        aiVector3D DiffuseColor = PropertyGet<aiVector3D>( props, baseName + "Color", ok );
        if ( ok ) {
            float DiffuseFactor = PropertyGet<float>( props, baseName + "Factor", ok );
            if ( ok ) {
                DiffuseColor *= DiffuseFactor;
            }

            return aiColor3D( DiffuseColor.x, DiffuseColor.y, DiffuseColor.z );
        }
    }
    result = false;
    return aiColor3D( 0.0f, 0.0f, 0.0f );
}


void Converter::SetShadingPropertiesCommon( aiMaterial* out_mat, const PropertyTable& props )
{
    // set shading properties. There are various, redundant ways in which FBX materials
    // specify their shading settings (depending on shading models, prop
    // template etc.). No idea which one is right in a particular context.
    // Just try to make sense of it - there's no spec to verify this against,
    // so why should we.
    bool ok;
    const aiColor3D& Diffuse = GetColorPropertyFromMaterial( props, "Diffuse", ok );
    if ( ok ) {
        out_mat->AddProperty( &Diffuse, 1, AI_MATKEY_COLOR_DIFFUSE );
    }

    const aiColor3D& Emissive = GetColorPropertyFromMaterial( props, "Emissive", ok );
    if ( ok ) {
        out_mat->AddProperty( &Emissive, 1, AI_MATKEY_COLOR_EMISSIVE );
    }

    const aiColor3D& Ambient = GetColorPropertyFromMaterial( props, "Ambient", ok );
    if ( ok ) {
        out_mat->AddProperty( &Ambient, 1, AI_MATKEY_COLOR_AMBIENT );
    }

    const aiColor3D& Specular = GetColorPropertyFromMaterial( props, "Specular", ok );
    if ( ok ) {
        out_mat->AddProperty( &Specular, 1, AI_MATKEY_COLOR_SPECULAR );
    }

    const float Opacity = PropertyGet<float>( props, "Opacity", ok );
    if ( ok ) {
        out_mat->AddProperty( &Opacity, 1, AI_MATKEY_OPACITY );
    }

    const float Reflectivity = PropertyGet<float>( props, "Reflectivity", ok );
    if ( ok ) {
        out_mat->AddProperty( &Reflectivity, 1, AI_MATKEY_REFLECTIVITY );
    }

    const float Shininess = PropertyGet<float>( props, "Shininess", ok );
    if ( ok ) {
        out_mat->AddProperty( &Shininess, 1, AI_MATKEY_SHININESS_STRENGTH );
    }

    const float ShininessExponent = PropertyGet<float>( props, "ShininessExponent", ok );
    if ( ok ) {
        out_mat->AddProperty( &ShininessExponent, 1, AI_MATKEY_SHININESS );
    }

    const float BumpFactor = PropertyGet<float>(props, "BumpFactor", ok);
    if (ok) {
        out_mat->AddProperty(&BumpFactor, 1, AI_MATKEY_BUMPSCALING);
    }

    const float DispFactor = PropertyGet<float>(props, "DisplacementFactor", ok);
    if (ok) {
        out_mat->AddProperty(&DispFactor, 1, "$mat.displacementscaling", 0, 0);
    }
}


double Converter::FrameRateToDouble( FileGlobalSettings::FrameRate fp, double customFPSVal )
{
    switch ( fp ) {
    case FileGlobalSettings::FrameRate_DEFAULT:
        return 1.0;

    case FileGlobalSettings::FrameRate_120:
        return 120.0;

    case FileGlobalSettings::FrameRate_100:
        return 100.0;

    case FileGlobalSettings::FrameRate_60:
        return 60.0;

    case FileGlobalSettings::FrameRate_50:
        return 50.0;

    case FileGlobalSettings::FrameRate_48:
        return 48.0;

    case FileGlobalSettings::FrameRate_30:
    case FileGlobalSettings::FrameRate_30_DROP:
        return 30.0;

    case FileGlobalSettings::FrameRate_NTSC_DROP_FRAME:
    case FileGlobalSettings::FrameRate_NTSC_FULL_FRAME:
        return 29.9700262;

    case FileGlobalSettings::FrameRate_PAL:
        return 25.0;

    case FileGlobalSettings::FrameRate_CINEMA:
        return 24.0;

    case FileGlobalSettings::FrameRate_1000:
        return 1000.0;

    case FileGlobalSettings::FrameRate_CINEMA_ND:
        return 23.976;

    case FileGlobalSettings::FrameRate_CUSTOM:
        return customFPSVal;

    case FileGlobalSettings::FrameRate_MAX: // this is to silence compiler warnings
        break;
    }

    ai_assert( false );
    return -1.0f;
}


void Converter::ConvertAnimations()
{
    // first of all determine framerate
    const FileGlobalSettings::FrameRate fps = doc.GlobalSettings().TimeMode();
    const float custom = doc.GlobalSettings().CustomFrameRate();
    anim_fps = FrameRateToDouble( fps, custom );

    const std::vector<const AnimationStack*>& animations = doc.AnimationStacks();
    for( const AnimationStack* stack : animations ) {
        ConvertAnimationStack( *stack );
    }
}

void Converter::RenameNode( const std::string& fixed_name, const std::string& new_name ) {
    if ( node_names.find( fixed_name ) == node_names.end() ) {
        FBXImporter::LogError( "Cannot rename node " + fixed_name + ", not existing.");
        return;
    }

    if ( node_names.find( new_name ) != node_names.end() ) {
        FBXImporter::LogError( "Cannot rename node " + fixed_name + " to " + new_name +", name already existing." );
        return;
    }

    ai_assert( node_names.find( fixed_name ) != node_names.end() );
    ai_assert( node_names.find( new_name ) == node_names.end() );

    renamed_nodes[ fixed_name ] = new_name;

    const aiString fn( fixed_name );

    for( aiCamera* cam : cameras ) {
        if ( cam->mName == fn ) {
            cam->mName.Set( new_name );
            break;
        }
    }

    for( aiLight* light : lights ) {
        if ( light->mName == fn ) {
            light->mName.Set( new_name );
            break;
        }
    }

    for( aiAnimation* anim : animations ) {
        for ( unsigned int i = 0; i < anim->mNumChannels; ++i ) {
            aiNodeAnim* const na = anim->mChannels[ i ];
            if ( na->mNodeName == fn ) {
                na->mNodeName.Set( new_name );
                break;
            }
        }
    }
}


std::string Converter::FixNodeName( const std::string& name )
{
    // strip Model:: prefix, avoiding ambiguities (i.e. don't strip if
    // this causes ambiguities, well possible between empty identifiers,
    // such as "Model::" and ""). Make sure the behaviour is consistent
    // across multiple calls to FixNodeName().
    if ( name.substr( 0, 7 ) == "Model::" ) {
        std::string temp = name.substr( 7 );

        const NodeNameMap::const_iterator it = node_names.find( temp );
        if ( it != node_names.end() ) {
            if ( !( *it ).second ) {
                return FixNodeName( name + "_" );
            }
        }
        node_names[ temp ] = true;

        const NameNameMap::const_iterator rit = renamed_nodes.find( temp );
        return rit == renamed_nodes.end() ? temp : ( *rit ).second;
    }

    const NodeNameMap::const_iterator it = node_names.find( name );
    if ( it != node_names.end() ) {
        if ( ( *it ).second ) {
            return FixNodeName( name + "_" );
        }
    }
    node_names[ name ] = false;

    const NameNameMap::const_iterator rit = renamed_nodes.find( name );
    return rit == renamed_nodes.end() ? name : ( *rit ).second;
}

void Converter::ConvertAnimationStack( const AnimationStack& st )
{
    const AnimationLayerList& layers = st.Layers();
    if ( layers.empty() ) {
        return;
    }

    aiAnimation* const anim = new aiAnimation();
    animations.push_back( anim );

    // strip AnimationStack:: prefix
    std::string name = st.Name();
    if ( name.substr( 0, 16 ) == "AnimationStack::" ) {
        name = name.substr( 16 );
    }
    else if ( name.substr( 0, 11 ) == "AnimStack::" ) {
        name = name.substr( 11 );
    }

    anim->mName.Set( name );

    // need to find all nodes for which we need to generate node animations -
    // it may happen that we need to merge multiple layers, though.
    NodeMap node_map;

    // reverse mapping from curves to layers, much faster than querying
    // the FBX DOM for it.
    LayerMap layer_map;

    const char* prop_whitelist[] = {
        "Lcl Scaling",
        "Lcl Rotation",
        "Lcl Translation"
    };

    for( const AnimationLayer* layer : layers ) {
        ai_assert( layer );

        const AnimationCurveNodeList& nodes = layer->Nodes( prop_whitelist, 3 );
        for( const AnimationCurveNode* node : nodes ) {
            ai_assert( node );

            const Model* const model = dynamic_cast<const Model*>( node->Target() );
            // this can happen - it could also be a NodeAttribute (i.e. for camera animations)
            if ( !model ) {
                continue;
            }

            const std::string& name = FixNodeName( model->Name() );
            node_map[ name ].push_back( node );

            layer_map[ node ] = layer;
        }
    }

    // generate node animations
    std::vector<aiNodeAnim*> node_anims;

    double min_time = 1e10;
    double max_time = -1e10;

    int64_t start_time = st.LocalStart();
    int64_t stop_time = st.LocalStop();
    bool has_local_startstop = start_time != 0 || stop_time != 0;
    if ( !has_local_startstop ) {
        // no time range given, so accept every keyframe and use the actual min/max time
        // the numbers are INT64_MIN/MAX, the 20000 is for safety because GenerateNodeAnimations uses an epsilon of 10000
        start_time = -9223372036854775807ll + 20000;
        stop_time = 9223372036854775807ll - 20000;
    }

    try {
        for( const NodeMap::value_type& kv : node_map ) {
            GenerateNodeAnimations( node_anims,
                kv.first,
                kv.second,
                layer_map,
                start_time, stop_time,
                max_time,
                min_time );
        }
    }
    catch ( std::exception& ) {
        std::for_each( node_anims.begin(), node_anims.end(), Util::delete_fun<aiNodeAnim>() );
        throw;
    }

    if ( node_anims.size() ) {
        anim->mChannels = new aiNodeAnim*[ node_anims.size() ]();
        anim->mNumChannels = static_cast<unsigned int>( node_anims.size() );

        std::swap_ranges( node_anims.begin(), node_anims.end(), anim->mChannels );
    }
    else {
        // empty animations would fail validation, so drop them
        delete anim;
        animations.pop_back();
        FBXImporter::LogInfo( "ignoring empty AnimationStack (using IK?): " + name );
        return;
    }

    double start_time_fps = has_local_startstop ? (CONVERT_FBX_TIME(start_time) * anim_fps) : min_time;
    double stop_time_fps = has_local_startstop ? (CONVERT_FBX_TIME(stop_time) * anim_fps) : max_time;

    // adjust relative timing for animation
    for ( unsigned int c = 0; c < anim->mNumChannels; c++ ) {
        aiNodeAnim* channel = anim->mChannels[ c ];
        for ( uint32_t i = 0; i < channel->mNumPositionKeys; i++ )
            channel->mPositionKeys[ i ].mTime -= start_time_fps;
        for ( uint32_t i = 0; i < channel->mNumRotationKeys; i++ )
            channel->mRotationKeys[ i ].mTime -= start_time_fps;
        for ( uint32_t i = 0; i < channel->mNumScalingKeys; i++ )
            channel->mScalingKeys[ i ].mTime -= start_time_fps;
    }

    // for some mysterious reason, mDuration is simply the maximum key -- the
    // validator always assumes animations to start at zero.
    anim->mDuration = stop_time_fps - start_time_fps;
    anim->mTicksPerSecond = anim_fps;
}

#ifdef ASSIMP_BUILD_DEBUG
// ------------------------------------------------------------------------------------------------
// sanity check whether the input is ok
static void validateAnimCurveNodes( const std::vector<const AnimationCurveNode*>& curves,
    bool strictMode ) {
    const Object* target( NULL );
    for( const AnimationCurveNode* node : curves ) {
        if ( !target ) {
            target = node->Target();
        }
        if ( node->Target() != target ) {
            FBXImporter::LogWarn( "Node target is nullptr type." );
        }
        if ( strictMode ) {
            ai_assert( node->Target() == target );
        }
    }
}
#endif // ASSIMP_BUILD_DEBUG

// ------------------------------------------------------------------------------------------------
void Converter::GenerateNodeAnimations( std::vector<aiNodeAnim*>& node_anims,
    const std::string& fixed_name,
    const std::vector<const AnimationCurveNode*>& curves,
    const LayerMap& layer_map,
    int64_t start, int64_t stop,
    double& max_time,
    double& min_time )
{

    NodeMap node_property_map;
    ai_assert( curves.size() );

#ifdef ASSIMP_BUILD_DEBUG
    validateAnimCurveNodes( curves, doc.Settings().strictMode );
#endif
    const AnimationCurveNode* curve_node = NULL;
    for( const AnimationCurveNode* node : curves ) {
        ai_assert( node );

        if ( node->TargetProperty().empty() ) {
            FBXImporter::LogWarn( "target property for animation curve not set: " + node->Name() );
            continue;
        }

        curve_node = node;
        if ( node->Curves().empty() ) {
            FBXImporter::LogWarn( "no animation curves assigned to AnimationCurveNode: " + node->Name() );
            continue;
        }

        node_property_map[ node->TargetProperty() ].push_back( node );
    }

    ai_assert( curve_node );
    ai_assert( curve_node->TargetAsModel() );

    const Model& target = *curve_node->TargetAsModel();

    // check for all possible transformation components
    NodeMap::const_iterator chain[ TransformationComp_MAXIMUM ];

    bool has_any = false;
    bool has_complex = false;

    for ( size_t i = 0; i < TransformationComp_MAXIMUM; ++i ) {
        const TransformationComp comp = static_cast<TransformationComp>( i );

        // inverse pivots don't exist in the input, we just generate them
        if ( comp == TransformationComp_RotationPivotInverse || comp == TransformationComp_ScalingPivotInverse ) {
            chain[ i ] = node_property_map.end();
            continue;
        }

        chain[ i ] = node_property_map.find( NameTransformationCompProperty( comp ) );
        if ( chain[ i ] != node_property_map.end() ) {

            // check if this curves contains redundant information by looking
            // up the corresponding node's transformation chain.
            if ( doc.Settings().optimizeEmptyAnimationCurves &&
                IsRedundantAnimationData( target, comp, ( *chain[ i ] ).second ) ) {

                FBXImporter::LogDebug( "dropping redundant animation channel for node " + target.Name() );
                continue;
            }

            has_any = true;

            if ( comp != TransformationComp_Rotation && comp != TransformationComp_Scaling && comp != TransformationComp_Translation &&
                comp != TransformationComp_GeometricScaling && comp != TransformationComp_GeometricRotation && comp != TransformationComp_GeometricTranslation )
            {
                has_complex = true;
            }
        }
    }

    if ( !has_any ) {
        FBXImporter::LogWarn( "ignoring node animation, did not find any transformation key frames" );
        return;
    }

    // this needs to play nicely with GenerateTransformationNodeChain() which will
    // be invoked _later_ (animations come first). If this node has only rotation,
    // scaling and translation _and_ there are no animated other components either,
    // we can use a single node and also a single node animation channel.
    if ( !has_complex && !NeedsComplexTransformationChain( target ) ) {

        aiNodeAnim* const nd = GenerateSimpleNodeAnim( fixed_name, target, chain,
            node_property_map.end(),
            layer_map,
            start, stop,
            max_time,
            min_time,
            true // input is TRS order, assimp is SRT
            );

        ai_assert( nd );
        if ( nd->mNumPositionKeys == 0 && nd->mNumRotationKeys == 0 && nd->mNumScalingKeys == 0 ) {
            delete nd;
        }
        else {
            node_anims.push_back( nd );
        }
        return;
    }

    // otherwise, things get gruesome and we need separate animation channels
    // for each part of the transformation chain. Remember which channels
    // we generated and pass this information to the node conversion
    // code to avoid nodes that have identity transform, but non-identity
    // animations, being dropped.
    unsigned int flags = 0, bit = 0x1;
    for ( size_t i = 0; i < TransformationComp_MAXIMUM; ++i, bit <<= 1 ) {
        const TransformationComp comp = static_cast<TransformationComp>( i );

        if ( chain[ i ] != node_property_map.end() ) {
            flags |= bit;

            ai_assert( comp != TransformationComp_RotationPivotInverse );
            ai_assert( comp != TransformationComp_ScalingPivotInverse );

            const std::string& chain_name = NameTransformationChainNode( fixed_name, comp );

            aiNodeAnim* na = nullptr;
            switch ( comp )
            {
            case TransformationComp_Rotation:
            case TransformationComp_PreRotation:
            case TransformationComp_PostRotation:
            case TransformationComp_GeometricRotation:
                na = GenerateRotationNodeAnim( chain_name,
                    target,
                    ( *chain[ i ] ).second,
                    layer_map,
                    start, stop,
                    max_time,
                    min_time );

                break;

            case TransformationComp_RotationOffset:
            case TransformationComp_RotationPivot:
            case TransformationComp_ScalingOffset:
            case TransformationComp_ScalingPivot:
            case TransformationComp_Translation:
            case TransformationComp_GeometricTranslation:
                na = GenerateTranslationNodeAnim( chain_name,
                    target,
                    ( *chain[ i ] ).second,
                    layer_map,
                    start, stop,
                    max_time,
                    min_time );

                // pivoting requires us to generate an implicit inverse channel to undo the pivot translation
                if ( comp == TransformationComp_RotationPivot ) {
                    const std::string& invName = NameTransformationChainNode( fixed_name,
                        TransformationComp_RotationPivotInverse );

                    aiNodeAnim* const inv = GenerateTranslationNodeAnim( invName,
                        target,
                        ( *chain[ i ] ).second,
                        layer_map,
                        start, stop,
                        max_time,
                        min_time,
                        true );

                    ai_assert( inv );
                    if ( inv->mNumPositionKeys == 0 && inv->mNumRotationKeys == 0 && inv->mNumScalingKeys == 0 ) {
                        delete inv;
                    }
                    else {
                        node_anims.push_back( inv );
                    }

                    ai_assert( TransformationComp_RotationPivotInverse > i );
                    flags |= bit << ( TransformationComp_RotationPivotInverse - i );
                }
                else if ( comp == TransformationComp_ScalingPivot ) {
                    const std::string& invName = NameTransformationChainNode( fixed_name,
                        TransformationComp_ScalingPivotInverse );

                    aiNodeAnim* const inv = GenerateTranslationNodeAnim( invName,
                        target,
                        ( *chain[ i ] ).second,
                        layer_map,
                        start, stop,
                        max_time,
                        min_time,
                        true );

                    ai_assert( inv );
                    if ( inv->mNumPositionKeys == 0 && inv->mNumRotationKeys == 0 && inv->mNumScalingKeys == 0 ) {
                        delete inv;
                    }
                    else {
                        node_anims.push_back( inv );
                    }

                    ai_assert( TransformationComp_RotationPivotInverse > i );
                    flags |= bit << ( TransformationComp_RotationPivotInverse - i );
                }

                break;

            case TransformationComp_Scaling:
            case TransformationComp_GeometricScaling:
                na = GenerateScalingNodeAnim( chain_name,
                    target,
                    ( *chain[ i ] ).second,
                    layer_map,
                    start, stop,
                    max_time,
                    min_time );

                break;

            default:
                ai_assert( false );
            }

            ai_assert( na );
            if ( na->mNumPositionKeys == 0 && na->mNumRotationKeys == 0 && na->mNumScalingKeys == 0 ) {
                delete na;
            }
            else {
                node_anims.push_back( na );
            }
            continue;
        }
    }

    node_anim_chain_bits[ fixed_name ] = flags;
}

bool Converter::IsRedundantAnimationData( const Model& target,
    TransformationComp comp,
    const std::vector<const AnimationCurveNode*>& curves )
{
    ai_assert( curves.size() );

    // look for animation nodes with
    //  * sub channels for all relevant components set
    //  * one key/value pair per component
    //  * combined values match up the corresponding value in the bind pose node transformation
    // only such nodes are 'redundant' for this function.

    if ( curves.size() > 1 ) {
        return false;
    }

    const AnimationCurveNode& nd = *curves.front();
    const AnimationCurveMap& sub_curves = nd.Curves();

    const AnimationCurveMap::const_iterator dx = sub_curves.find( "d|X" );
    const AnimationCurveMap::const_iterator dy = sub_curves.find( "d|Y" );
    const AnimationCurveMap::const_iterator dz = sub_curves.find( "d|Z" );

    if ( dx == sub_curves.end() || dy == sub_curves.end() || dz == sub_curves.end() ) {
        return false;
    }

    const KeyValueList& vx = ( *dx ).second->GetValues();
    const KeyValueList& vy = ( *dy ).second->GetValues();
    const KeyValueList& vz = ( *dz ).second->GetValues();

    if ( vx.size() != 1 || vy.size() != 1 || vz.size() != 1 ) {
        return false;
    }

    const aiVector3D dyn_val = aiVector3D( vx[ 0 ], vy[ 0 ], vz[ 0 ] );
    const aiVector3D& static_val = PropertyGet<aiVector3D>( target.Props(),
        NameTransformationCompProperty( comp ),
        TransformationCompDefaultValue( comp )
        );

    const float epsilon = 1e-6f;
    return ( dyn_val - static_val ).SquareLength() < epsilon;
}


aiNodeAnim* Converter::GenerateRotationNodeAnim( const std::string& name,
    const Model& target,
    const std::vector<const AnimationCurveNode*>& curves,
    const LayerMap& layer_map,
    int64_t start, int64_t stop,
    double& max_time,
    double& min_time )
{
    std::unique_ptr<aiNodeAnim> na( new aiNodeAnim() );
    na->mNodeName.Set( name );

    ConvertRotationKeys( na.get(), curves, layer_map, start, stop, max_time, min_time, target.RotationOrder() );

    // dummy scaling key
    na->mScalingKeys = new aiVectorKey[ 1 ];
    na->mNumScalingKeys = 1;

    na->mScalingKeys[ 0 ].mTime = 0.;
    na->mScalingKeys[ 0 ].mValue = aiVector3D( 1.0f, 1.0f, 1.0f );

    // dummy position key
    na->mPositionKeys = new aiVectorKey[ 1 ];
    na->mNumPositionKeys = 1;

    na->mPositionKeys[ 0 ].mTime = 0.;
    na->mPositionKeys[ 0 ].mValue = aiVector3D();

    return na.release();
}

aiNodeAnim* Converter::GenerateScalingNodeAnim( const std::string& name,
    const Model& /*target*/,
    const std::vector<const AnimationCurveNode*>& curves,
    const LayerMap& layer_map,
    int64_t start, int64_t stop,
    double& max_time,
    double& min_time )
{
    std::unique_ptr<aiNodeAnim> na( new aiNodeAnim() );
    na->mNodeName.Set( name );

    ConvertScaleKeys( na.get(), curves, layer_map, start, stop, max_time, min_time );

    // dummy rotation key
    na->mRotationKeys = new aiQuatKey[ 1 ];
    na->mNumRotationKeys = 1;

    na->mRotationKeys[ 0 ].mTime = 0.;
    na->mRotationKeys[ 0 ].mValue = aiQuaternion();

    // dummy position key
    na->mPositionKeys = new aiVectorKey[ 1 ];
    na->mNumPositionKeys = 1;

    na->mPositionKeys[ 0 ].mTime = 0.;
    na->mPositionKeys[ 0 ].mValue = aiVector3D();

    return na.release();
}


aiNodeAnim* Converter::GenerateTranslationNodeAnim( const std::string& name,
    const Model& /*target*/,
    const std::vector<const AnimationCurveNode*>& curves,
    const LayerMap& layer_map,
    int64_t start, int64_t stop,
    double& max_time,
    double& min_time,
    bool inverse )
{
    std::unique_ptr<aiNodeAnim> na( new aiNodeAnim() );
    na->mNodeName.Set( name );

    ConvertTranslationKeys( na.get(), curves, layer_map, start, stop, max_time, min_time );

    if ( inverse ) {
        for ( unsigned int i = 0; i < na->mNumPositionKeys; ++i ) {
            na->mPositionKeys[ i ].mValue *= -1.0f;
        }
    }

    // dummy scaling key
    na->mScalingKeys = new aiVectorKey[ 1 ];
    na->mNumScalingKeys = 1;

    na->mScalingKeys[ 0 ].mTime = 0.;
    na->mScalingKeys[ 0 ].mValue = aiVector3D( 1.0f, 1.0f, 1.0f );

    // dummy rotation key
    na->mRotationKeys = new aiQuatKey[ 1 ];
    na->mNumRotationKeys = 1;

    na->mRotationKeys[ 0 ].mTime = 0.;
    na->mRotationKeys[ 0 ].mValue = aiQuaternion();

    return na.release();
}

aiNodeAnim* Converter::GenerateSimpleNodeAnim( const std::string& name,
    const Model& target,
    NodeMap::const_iterator chain[ TransformationComp_MAXIMUM ],
    NodeMap::const_iterator iter_end,
    const LayerMap& layer_map,
    int64_t start, int64_t stop,
    double& max_time,
    double& min_time,
    bool reverse_order )

{
    std::unique_ptr<aiNodeAnim> na( new aiNodeAnim() );
    na->mNodeName.Set( name );

    const PropertyTable& props = target.Props();

    // need to convert from TRS order to SRT?
    if ( reverse_order ) {

        aiVector3D def_scale = PropertyGet( props, "Lcl Scaling", aiVector3D( 1.f, 1.f, 1.f ) );
        aiVector3D def_translate = PropertyGet( props, "Lcl Translation", aiVector3D( 0.f, 0.f, 0.f ) );
        aiVector3D def_rot = PropertyGet( props, "Lcl Rotation", aiVector3D( 0.f, 0.f, 0.f ) );

        KeyFrameListList scaling;
        KeyFrameListList translation;
        KeyFrameListList rotation;

        if ( chain[ TransformationComp_Scaling ] != iter_end ) {
            scaling = GetKeyframeList( ( *chain[ TransformationComp_Scaling ] ).second, start, stop );
        }

        if ( chain[ TransformationComp_Translation ] != iter_end ) {
            translation = GetKeyframeList( ( *chain[ TransformationComp_Translation ] ).second, start, stop );
        }

        if ( chain[ TransformationComp_Rotation ] != iter_end ) {
            rotation = GetKeyframeList( ( *chain[ TransformationComp_Rotation ] ).second, start, stop );
        }

        KeyFrameListList joined;
        joined.insert( joined.end(), scaling.begin(), scaling.end() );
        joined.insert( joined.end(), translation.begin(), translation.end() );
        joined.insert( joined.end(), rotation.begin(), rotation.end() );

        const KeyTimeList& times = GetKeyTimeList( joined );

        aiQuatKey* out_quat = new aiQuatKey[ times.size() ];
        aiVectorKey* out_scale = new aiVectorKey[ times.size() ];
        aiVectorKey* out_translation = new aiVectorKey[ times.size() ];

        if ( times.size() )
        {
            ConvertTransformOrder_TRStoSRT( out_quat, out_scale, out_translation,
                scaling,
                translation,
                rotation,
                times,
                max_time,
                min_time,
                target.RotationOrder(),
                def_scale,
                def_translate,
                def_rot );
        }

        // XXX remove duplicates / redundant keys which this operation did
        // likely produce if not all three channels were equally dense.

        na->mNumScalingKeys = static_cast<unsigned int>( times.size() );
        na->mNumRotationKeys = na->mNumScalingKeys;
        na->mNumPositionKeys = na->mNumScalingKeys;

        na->mScalingKeys = out_scale;
        na->mRotationKeys = out_quat;
        na->mPositionKeys = out_translation;
    }
    else {

        // if a particular transformation is not given, grab it from
        // the corresponding node to meet the semantics of aiNodeAnim,
        // which requires all of rotation, scaling and translation
        // to be set.
        if ( chain[ TransformationComp_Scaling ] != iter_end ) {
            ConvertScaleKeys( na.get(), ( *chain[ TransformationComp_Scaling ] ).second,
                layer_map,
                start, stop,
                max_time,
                min_time );
        }
        else {
            na->mScalingKeys = new aiVectorKey[ 1 ];
            na->mNumScalingKeys = 1;

            na->mScalingKeys[ 0 ].mTime = 0.;
            na->mScalingKeys[ 0 ].mValue = PropertyGet( props, "Lcl Scaling",
                aiVector3D( 1.f, 1.f, 1.f ) );
        }

        if ( chain[ TransformationComp_Rotation ] != iter_end ) {
            ConvertRotationKeys( na.get(), ( *chain[ TransformationComp_Rotation ] ).second,
                layer_map,
                start, stop,
                max_time,
                min_time,
                target.RotationOrder() );
        }
        else {
            na->mRotationKeys = new aiQuatKey[ 1 ];
            na->mNumRotationKeys = 1;

            na->mRotationKeys[ 0 ].mTime = 0.;
            na->mRotationKeys[ 0 ].mValue = EulerToQuaternion(
                PropertyGet( props, "Lcl Rotation", aiVector3D( 0.f, 0.f, 0.f ) ),
                target.RotationOrder() );
        }

        if ( chain[ TransformationComp_Translation ] != iter_end ) {
            ConvertTranslationKeys( na.get(), ( *chain[ TransformationComp_Translation ] ).second,
                layer_map,
                start, stop,
                max_time,
                min_time );
        }
        else {
            na->mPositionKeys = new aiVectorKey[ 1 ];
            na->mNumPositionKeys = 1;

            na->mPositionKeys[ 0 ].mTime = 0.;
            na->mPositionKeys[ 0 ].mValue = PropertyGet( props, "Lcl Translation",
                aiVector3D( 0.f, 0.f, 0.f ) );
        }

    }
    return na.release();
}

Converter::KeyFrameListList Converter::GetKeyframeList( const std::vector<const AnimationCurveNode*>& nodes, int64_t start, int64_t stop )
{
    KeyFrameListList inputs;
    inputs.reserve( nodes.size() * 3 );

    //give some breathing room for rounding errors
    int64_t adj_start = start - 10000;
    int64_t adj_stop = stop + 10000;

    for( const AnimationCurveNode* node : nodes ) {
        ai_assert( node );

        const AnimationCurveMap& curves = node->Curves();
        for( const AnimationCurveMap::value_type& kv : curves ) {

            unsigned int mapto;
            if ( kv.first == "d|X" ) {
                mapto = 0;
            }
            else if ( kv.first == "d|Y" ) {
                mapto = 1;
            }
            else if ( kv.first == "d|Z" ) {
                mapto = 2;
            }
            else {
                FBXImporter::LogWarn( "ignoring scale animation curve, did not recognize target component" );
                continue;
            }

            const AnimationCurve* const curve = kv.second;
            ai_assert( curve->GetKeys().size() == curve->GetValues().size() && curve->GetKeys().size() );

            //get values within the start/stop time window
            std::shared_ptr<KeyTimeList> Keys( new KeyTimeList() );
            std::shared_ptr<KeyValueList> Values( new KeyValueList() );
            const size_t count = curve->GetKeys().size();
            Keys->reserve( count );
            Values->reserve( count );
            for (size_t n = 0; n < count; n++ )
            {
                int64_t k = curve->GetKeys().at( n );
                if ( k >= adj_start && k <= adj_stop )
                {
                    Keys->push_back( k );
                    Values->push_back( curve->GetValues().at( n ) );
                }
            }

            inputs.push_back( std::make_tuple( Keys, Values, mapto ) );
        }
    }
    return inputs; // pray for NRVO :-)
}


KeyTimeList Converter::GetKeyTimeList( const KeyFrameListList& inputs )
{
    ai_assert( inputs.size() );

    // reserve some space upfront - it is likely that the keyframe lists
    // have matching time values, so max(of all keyframe lists) should
    // be a good estimate.
    KeyTimeList keys;

    size_t estimate = 0;
    for( const KeyFrameList& kfl : inputs ) {
        estimate = std::max( estimate, std::get<0>(kfl)->size() );
    }

    keys.reserve( estimate );

    std::vector<unsigned int> next_pos;
    next_pos.resize( inputs.size(), 0 );

    const size_t count = inputs.size();
    while ( true ) {

        int64_t min_tick = std::numeric_limits<int64_t>::max();
        for ( size_t i = 0; i < count; ++i ) {
            const KeyFrameList& kfl = inputs[ i ];

            if ( std::get<0>(kfl)->size() > next_pos[ i ] && std::get<0>(kfl)->at( next_pos[ i ] ) < min_tick ) {
                min_tick = std::get<0>(kfl)->at( next_pos[ i ] );
            }
        }

        if ( min_tick == std::numeric_limits<int64_t>::max() ) {
            break;
        }
        keys.push_back( min_tick );

        for ( size_t i = 0; i < count; ++i ) {
            const KeyFrameList& kfl = inputs[ i ];


            while ( std::get<0>(kfl)->size() > next_pos[ i ] && std::get<0>(kfl)->at( next_pos[ i ] ) == min_tick ) {
                ++next_pos[ i ];
            }
        }
    }

    return keys;
}

void Converter::InterpolateKeys( aiVectorKey* valOut, const KeyTimeList& keys, const KeyFrameListList& inputs,
    const aiVector3D& def_value,
    double& max_time,
    double& min_time )

{
    ai_assert( keys.size() );
    ai_assert( valOut );

    std::vector<unsigned int> next_pos;
    const size_t count = inputs.size();

    next_pos.resize( inputs.size(), 0 );

    for( KeyTimeList::value_type time : keys ) {
        ai_real result[ 3 ] = { def_value.x, def_value.y, def_value.z };

        for ( size_t i = 0; i < count; ++i ) {
            const KeyFrameList& kfl = inputs[ i ];

            const size_t ksize = std::get<0>(kfl)->size();
            if ( ksize > next_pos[ i ] && std::get<0>(kfl)->at( next_pos[ i ] ) == time ) {
                ++next_pos[ i ];
            }

            const size_t id0 = next_pos[ i ]>0 ? next_pos[ i ] - 1 : 0;
            const size_t id1 = next_pos[ i ] == ksize ? ksize - 1 : next_pos[ i ];

            // use lerp for interpolation
            const KeyValueList::value_type valueA = std::get<1>(kfl)->at( id0 );
            const KeyValueList::value_type valueB = std::get<1>(kfl)->at( id1 );

            const KeyTimeList::value_type timeA = std::get<0>(kfl)->at( id0 );
            const KeyTimeList::value_type timeB = std::get<0>(kfl)->at( id1 );

            const ai_real factor = timeB == timeA ? ai_real(0.) : static_cast<ai_real>( ( time - timeA ) ) / ( timeB - timeA );
            const ai_real interpValue = static_cast<ai_real>( valueA + ( valueB - valueA ) * factor );

            result[ std::get<2>(kfl) ] = interpValue;
        }

        // magic value to convert fbx times to seconds
        valOut->mTime = CONVERT_FBX_TIME( time ) * anim_fps;

        min_time = std::min( min_time, valOut->mTime );
        max_time = std::max( max_time, valOut->mTime );

        valOut->mValue.x = result[ 0 ];
        valOut->mValue.y = result[ 1 ];
        valOut->mValue.z = result[ 2 ];

        ++valOut;
    }
}

void Converter::InterpolateKeys( aiQuatKey* valOut, const KeyTimeList& keys, const KeyFrameListList& inputs,
    const aiVector3D& def_value,
    double& maxTime,
    double& minTime,
    Model::RotOrder order )
{
    ai_assert( keys.size() );
    ai_assert( valOut );

    std::unique_ptr<aiVectorKey[]> temp( new aiVectorKey[ keys.size() ] );
    InterpolateKeys( temp.get(), keys, inputs, def_value, maxTime, minTime );

    aiMatrix4x4 m;

    aiQuaternion lastq;

    for ( size_t i = 0, c = keys.size(); i < c; ++i ) {

        valOut[ i ].mTime = temp[ i ].mTime;

        GetRotationMatrix( order, temp[ i ].mValue, m );
        aiQuaternion quat = aiQuaternion( aiMatrix3x3( m ) );

        // take shortest path by checking the inner product
        // http://www.3dkingdoms.com/weekly/weekly.php?a=36
        if ( quat.x * lastq.x + quat.y * lastq.y + quat.z * lastq.z + quat.w * lastq.w < 0 )
        {
            quat.x = -quat.x;
            quat.y = -quat.y;
            quat.z = -quat.z;
            quat.w = -quat.w;
        }
        lastq = quat;

        valOut[ i ].mValue = quat;
    }
}

void Converter::ConvertTransformOrder_TRStoSRT( aiQuatKey* out_quat, aiVectorKey* out_scale,
    aiVectorKey* out_translation,
    const KeyFrameListList& scaling,
    const KeyFrameListList& translation,
    const KeyFrameListList& rotation,
    const KeyTimeList& times,
    double& maxTime,
    double& minTime,
    Model::RotOrder order,
    const aiVector3D& def_scale,
    const aiVector3D& def_translate,
    const aiVector3D& def_rotation )
{
    if ( rotation.size() ) {
        InterpolateKeys( out_quat, times, rotation, def_rotation, maxTime, minTime, order );
    }
    else {
        for ( size_t i = 0; i < times.size(); ++i ) {
            out_quat[ i ].mTime = CONVERT_FBX_TIME( times[ i ] ) * anim_fps;
            out_quat[ i ].mValue = EulerToQuaternion( def_rotation, order );
        }
    }

    if ( scaling.size() ) {
        InterpolateKeys( out_scale, times, scaling, def_scale, maxTime, minTime );
    }
    else {
        for ( size_t i = 0; i < times.size(); ++i ) {
            out_scale[ i ].mTime = CONVERT_FBX_TIME( times[ i ] ) * anim_fps;
            out_scale[ i ].mValue = def_scale;
        }
    }

    if ( translation.size() ) {
        InterpolateKeys( out_translation, times, translation, def_translate, maxTime, minTime );
    }
    else {
        for ( size_t i = 0; i < times.size(); ++i ) {
            out_translation[ i ].mTime = CONVERT_FBX_TIME( times[ i ] ) * anim_fps;
            out_translation[ i ].mValue = def_translate;
        }
    }

    const size_t count = times.size();
    for ( size_t i = 0; i < count; ++i ) {
        aiQuaternion& r = out_quat[ i ].mValue;
        aiVector3D& s = out_scale[ i ].mValue;
        aiVector3D& t = out_translation[ i ].mValue;

        aiMatrix4x4 mat, temp;
        aiMatrix4x4::Translation( t, mat );
        mat *= aiMatrix4x4( r.GetMatrix() );
        mat *= aiMatrix4x4::Scaling( s, temp );

        mat.Decompose( s, r, t );
    }
}

aiQuaternion Converter::EulerToQuaternion( const aiVector3D& rot, Model::RotOrder order )
{
    aiMatrix4x4 m;
    GetRotationMatrix( order, rot, m );

    return aiQuaternion( aiMatrix3x3( m ) );
}

void Converter::ConvertScaleKeys( aiNodeAnim* na, const std::vector<const AnimationCurveNode*>& nodes, const LayerMap& /*layers*/,
    int64_t start, int64_t stop,
    double& maxTime,
    double& minTime )
{
    ai_assert( nodes.size() );

    // XXX for now, assume scale should be blended geometrically (i.e. two
    // layers should be multiplied with each other). There is a FBX
    // property in the layer to specify the behaviour, though.

    const KeyFrameListList& inputs = GetKeyframeList( nodes, start, stop );
    const KeyTimeList& keys = GetKeyTimeList( inputs );

    na->mNumScalingKeys = static_cast<unsigned int>( keys.size() );
    na->mScalingKeys = new aiVectorKey[ keys.size() ];
    if ( keys.size() > 0 )
        InterpolateKeys( na->mScalingKeys, keys, inputs, aiVector3D( 1.0f, 1.0f, 1.0f ), maxTime, minTime );
}

void Converter::ConvertTranslationKeys( aiNodeAnim* na, const std::vector<const AnimationCurveNode*>& nodes,
    const LayerMap& /*layers*/,
    int64_t start, int64_t stop,
    double& maxTime,
    double& minTime )
{
    ai_assert( nodes.size() );

    // XXX see notes in ConvertScaleKeys()
    const KeyFrameListList& inputs = GetKeyframeList( nodes, start, stop );
    const KeyTimeList& keys = GetKeyTimeList( inputs );

    na->mNumPositionKeys = static_cast<unsigned int>( keys.size() );
    na->mPositionKeys = new aiVectorKey[ keys.size() ];
    if ( keys.size() > 0 )
        InterpolateKeys( na->mPositionKeys, keys, inputs, aiVector3D( 0.0f, 0.0f, 0.0f ), maxTime, minTime );
}

void Converter::ConvertRotationKeys( aiNodeAnim* na, const std::vector<const AnimationCurveNode*>& nodes,
    const LayerMap& /*layers*/,
    int64_t start, int64_t stop,
    double& maxTime,
    double& minTime,
    Model::RotOrder order )
{
    ai_assert( nodes.size() );

    // XXX see notes in ConvertScaleKeys()
    const std::vector< KeyFrameList >& inputs = GetKeyframeList( nodes, start, stop );
    const KeyTimeList& keys = GetKeyTimeList( inputs );

    na->mNumRotationKeys = static_cast<unsigned int>( keys.size() );
    na->mRotationKeys = new aiQuatKey[ keys.size() ];
    if ( keys.size() > 0 )
        InterpolateKeys( na->mRotationKeys, keys, inputs, aiVector3D( 0.0f, 0.0f, 0.0f ), maxTime, minTime, order );
}

void Converter::TransferDataToScene()
{
    ai_assert( !out->mMeshes );
    ai_assert( !out->mNumMeshes );

    // note: the trailing () ensures initialization with NULL - not
    // many C++ users seem to know this, so pointing it out to avoid
    // confusion why this code works.

    if ( meshes.size() ) {
        out->mMeshes = new aiMesh*[ meshes.size() ]();
        out->mNumMeshes = static_cast<unsigned int>( meshes.size() );

        std::swap_ranges( meshes.begin(), meshes.end(), out->mMeshes );
    }

    if ( materials.size() ) {
        out->mMaterials = new aiMaterial*[ materials.size() ]();
        out->mNumMaterials = static_cast<unsigned int>( materials.size() );

        std::swap_ranges( materials.begin(), materials.end(), out->mMaterials );
    }

    if ( animations.size() ) {
        out->mAnimations = new aiAnimation*[ animations.size() ]();
        out->mNumAnimations = static_cast<unsigned int>( animations.size() );

        std::swap_ranges( animations.begin(), animations.end(), out->mAnimations );
    }

    if ( lights.size() ) {
        out->mLights = new aiLight*[ lights.size() ]();
        out->mNumLights = static_cast<unsigned int>( lights.size() );

        std::swap_ranges( lights.begin(), lights.end(), out->mLights );
    }

    if ( cameras.size() ) {
        out->mCameras = new aiCamera*[ cameras.size() ]();
        out->mNumCameras = static_cast<unsigned int>( cameras.size() );

        std::swap_ranges( cameras.begin(), cameras.end(), out->mCameras );
    }

    if ( textures.size() ) {
        out->mTextures = new aiTexture*[ textures.size() ]();
        out->mNumTextures = static_cast<unsigned int>( textures.size() );

        std::swap_ranges( textures.begin(), textures.end(), out->mTextures );
    }
}

//} // !anon

// ------------------------------------------------------------------------------------------------
void ConvertToAssimpScene(aiScene* out, const Document& doc)
{
    Converter converter(out,doc);
}

} // !FBX
} // !Assimp

#endif
