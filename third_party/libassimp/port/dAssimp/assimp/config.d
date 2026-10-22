/*
---------------------------------------------------------------------------
Open Asset Import Library (ASSIMP)
---------------------------------------------------------------------------

Copyright (c) 2006-2009, ASSIMP Development Team

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

 * Neither the name of the ASSIMP team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the ASSIMP Development Team.

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

/**
 * Defines constants for configurable properties for the library.
 *
 * These are set via <code>aiSetImportPropertyInteger()</code>,
 * <code>aiSetImportPropertyFloat()</code> and
 * <code>aiSetImportPropertyString()</code>.
 */
module assimp.config;

extern ( C ) {
   /*
    * Library settings.
    *
    * General, global settings.
    */

   /**
    * Enables time measurements.
    *
    * If enabled, measures the time needed for each part of the loading
    * process (i.e. IO time, importing, postprocessing, ..) and dumps these
    * timings to the DefaultLogger. See the performance page in the main
    * Assimp docs information on this topic.
    *
    * Property type: bool. Default value: false.
    */
   const char* AI_CONFIG_GLOB_MEASURE_TIME = "GLOB_MEASURE_TIME";

   version( none ) { // not implemented yet
   /**
    * Set Assimp's multithreading policy.
    *
    * This setting is ignored if Assimp was built without boost.thread support
    * (<code>ASSIMP_BUILD_NO_THREADING</code>, which is implied by
    * <code>ASSIMP_BUILD_BOOST_WORKAROUND</code>).
    *
    * Possible values are: -1 to let Assimp decide what to do, 0 to disable
    * multithreading entirely and any number larger than 0 to force a specific
    * number of threads. Assimp is always free to ignore this settings, which
    * is merely a hint. Usually, the default value (-1) will be fine. However,
    * if Assimp is used concurrently from multiple user threads, it might be
    * useful to limit each Importer instance to a specific number of cores.
    *
    * For more information, see the threading page in the main Assimp docs.
    *
    * Property type: int, default value: -1.
    */
   const char* AI_CONFIG_GLOB_MULTITHREADING = "GLOB_MULTITHREADING";
   }


   /*
    * Post processing settings.
    *
    * Various options to fine-tune the behavior of a specific post processing step.
    */

   /**
    * Specifies the maximum angle that may be between two vertex tangents that
    * their tangents and bitangents are smoothed.
    *
    * This applies to the <code>CalcTangentSpace</code> step. The angle is
    * specified in degrees, so 180 corresponds to PI radians.
    *
    * The default value is 45, the maximum value is 175.
    *
    * Property type: float.
    */
   const char* AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE = "PP_CT_MAX_SMOOTHING_ANGLE";

   /**
    * Specifies the maximum angle that may be between two face normals at the
    * same vertex position that their are smoothed together. Sometimes referred
    * to as 'crease angle'.
    *
    * This applies to the <code>GenSmoothNormals</code> step. The angle is
    * specified in degrees, so 180 corresponds to PI radians.
    *
    * The default value is 175 degrees (all vertex normals are smoothed), the
    * maximum value is 175, too.
    *
    * Property type: float.
    *
    * Warning:
    *    Setting this option may cause a severe loss of performance. The
    *    performance is unaffected if the <code>AI_CONFIG_FAVOUR_SPEED</code>
    *    flag is set but the output quality may be reduced.
    */
   const char* AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE = "PP_GSN_MAX_SMOOTHING_ANGLE";

   /**
    * Sets the colormap (= palette) to be used to decode embedded textures in
    * MDL (Quake or 3DGS) files.
    *
    * This must be a valid path to a file. The file is 768 (256*3) bytes large
    * and contains RGB triplets for each of the 256 palette entries. The
    * default value is colormap.lmp. If the file is not found, a default
    * palette (from Quake 1) is used.
    *
    * Property type: string.
    */
   const char* AI_CONFIG_IMPORT_MDL_COLORMAP = "IMPORT_MDL_COLORMAP";

   /**
    * Configures the <code>RemoveRedundantMaterials</code> step to keep
    * materials matching a name in a given list.
    *
    * This is a list of 1 to n strings, ' ' serves as delimiter character.
    * Identifiers containing whitespaces must be enclosed in <em>single</em>
    * quotation marks. For example: <code>
    * "keep-me and_me_to anotherMaterialToBeKept \'name with whitespace\'"</code>.
    * Linefeeds, tabs or carriage returns are treated as whitespace.
    *
    * If a material matches on of these names, it will not be modified or
    * removed by the postprocessing step nor will other materials be replaced
    * by a reference to it.
    *
    * This option might be useful if you are using some magic material names
    * to pass additional semantics through the content pipeline. This ensures
    * they won't be optimized away, but a general optimization is still
    * performed for materials not contained in the list.
    *
    * Default value: n/a
    *
    * Property type: string.
    *
    * Note: Material names are case sensitive.
    */
   const char* AI_CONFIG_PP_RRM_EXCLUDE_LIST = "PP_RRM_EXCLUDE_LIST";

   /**
    * Configures the <code>PretransformVertices</code> step to keep the scene
    * hierarchy. Meshes are moved to worldspace, but no optimization is
    * performed (meshes with equal materials are not joined, the total number
    * of meshes will not change).
    *
    * This option could be of use for you if the scene hierarchy contains
    * important additional information which you intend to parse.
    * For rendering, you can still render all meshes in the scene without
    * any transformations.
    *
    * Default value: false.
    *
    * Property type: bool.
    */
   const char* AI_CONFIG_PP_PTV_KEEP_HIERARCHY = "PP_PTV_KEEP_HIERARCHY";

   /**
    * Configures the <code>PretransformVertices</code> step to normalize all
    * vertex components into the -1...1 range. That is, a bounding box for the
    * whole scene is computed, the maximum component is taken and all meshes
    * are scaled appropriately (uniformly of course!).
    *
    * This might be useful if you don't know the spatial dimension of the input
    * data.
    */
   const char* AI_CONFIG_PP_PTV_NORMALIZE = "PP_PTV_NORMALIZE";

   /**
    * Configures the <code>FindDegenerates</code> step to remove degenerated
    * primitives from the import – immediately.
    *
    * The default behaviour converts degenerated triangles to lines and
    * degenerated lines to points. See the documentation to the
    * <code>FindDegenerates</code> step for a detailed example of the various
    * ways to get rid of these lines and points if you don't want them.
    *
    * Default value: false.
    *
    * Property type: bool.
    */
   const char* AI_CONFIG_PP_FD_REMOVE = "PP_FD_REMOVE";

   /**
    * Configures the <code>OptimizeGraph</code> step to preserve nodes matching
    * a name in a given list.
    *
    * This is a list of 1 to n strings, ' ' serves as delimiter character.
    * Identifiers containing whitespaces must be enclosed in <em>single</em>
    * quotation marks. For example: <code>
    * "keep-me and_me_to anotherMaterialToBeKept \'name with whitespace\'"</code>.
    * Linefeeds, tabs or carriage returns are treated as whitespace.
    *
    * If a node matches on of these names, it will not be modified or
    * removed by the postprocessing step.
    *
    * This option might be useful if you are using some magic node names
    * to pass additional semantics through the content pipeline. This ensures
    * they won't be optimized away, but a general optimization is still
    * performed for nodes not contained in the list.
    *
    * Default value: n/a
    *
    * Property type: string.
    *
    * Note: Node names are case sensitive.
    */
   const char* AI_CONFIG_PP_OG_EXCLUDE_LIST = "PP_OG_EXCLUDE_LIST";

   /**
    * Sets the maximum number of triangles in a mesh.
    *
    * This is used by the <code>SplitLargeMeshes</code> step to determine
    * whether a mesh must be split or not.
    *
    * Default value: AI_SLM_DEFAULT_MAX_TRIANGLES.
    *
    * Property type: integer.
    */
   const char* AI_CONFIG_PP_SLM_TRIANGLE_LIMIT = "PP_SLM_TRIANGLE_LIMIT";

   /**
    * The default value for the AI_CONFIG_PP_SLM_TRIANGLE_LIMIT setting.
    */
   const AI_SLM_DEFAULT_MAX_TRIANGLES = 1000000;

   /**
    * Sets the maximum number of vertices in a mesh.
    *
    * This is used by the <code>SplitLargeMeshes</code> step to determine
    * whether a mesh must be split or not.
    *
    * Default value: AI_SLM_DEFAULT_MAX_VERTICES
    *
    * Property type: integer.
    */
   const char* AI_CONFIG_PP_SLM_VERTEX_LIMIT = "PP_SLM_VERTEX_LIMIT";

   /**
    * The default value for the AI_CONFIG_PP_SLM_VERTEX_LIMIT setting.
    */
   const AI_SLM_DEFAULT_MAX_VERTICES = 1000000;

   /**
    * Sets the maximum number of bones affecting a single vertex.
    *
    * This is used by the <code>LimitBoneWeights</code> step.
    *
    * Default value: AI_LBW_MAX_WEIGHTS
    *
    * Property type: integer.
    */
   const char* AI_CONFIG_PP_LBW_MAX_WEIGHTS = "PP_LBW_MAX_WEIGHTS";

   /**
    * The default value for the AI_CONFIG_PP_LBW_MAX_WEIGHTS setting.
    */
   const AI_LMW_MAX_WEIGHTS = 0x4;

   /**
    * Sets the size of the post-transform vertex cache to optimize the
    * vertices for. This configures the <code>ImproveCacheLocality</code> step.
    *
    * The size is given in vertices. Of course you can't know how the vertex
    * format will exactly look like after the import returns, but you can still
    * guess what your meshes will probably have.
    *
    * The default value results in slight performance improvements for most
    * nVidia/AMD cards since 2002.
    *
    * Default value: PP_ICL_PTCACHE_SIZE
    *
    * Property type: integer.
    */
   const char* AI_CONFIG_PP_ICL_PTCACHE_SIZE = "PP_ICL_PTCACHE_SIZE";

   /**
    * The default value for the AI_CONFIG_PP_ICL_PTCACHE_SIZE config option.
    */
   const PP_ICL_PTCACHE_SIZE = 12;

   /**
    * Components of the <code>aiScene</code> and <code>aiMesh</code> data
    * structures that can be excluded from the import by using the
    * <code>RemoveComponent</code> step.
    *
    *  See the documentation to <code>RemoveComponent</code> for more details.
    */
   enum aiComponent : uint {
      /**
       * Normal vectors.
       */
      NORMALS = 0x2,

      /**
       * Tangents and bitangents.
       */
      TANGENTS_AND_BITANGENTS = 0x4,

      /**
       * <em>All</em> color sets.
       *
       * Use aiComponent_COLORSn( N ) to specify the N'th set.
       */
      COLORS = 0x8,

      /**
       * <em>All</em> texture UV coordinate sets.
       *
       * Use aiComponent_TEXCOORDn( N ) to specify the N'th set.
       */
      TEXCOORDS = 0x10,

      /**
       * Bone weights from all meshes.
       *
       * The corresponding scenegraph nodes are <em>not</em> removed. Use the
       * <code>OptimizeGraph</code> step to do this.
       */
      BONEWEIGHTS = 0x20,

      /**
       * Node animations (<code>aiScene.mAnimations</code>).
       *
       * The corresponding scenegraph nodes are <em>not</em> removed. Use the
       * <code>OptimizeGraph</code> step to do this.
       */
      ANIMATIONS = 0x40,

      /**
       * Embedded textures (<code>aiScene.mTextures</code>).
      */
      TEXTURES = 0x80,

      /**
       * Light sources (<code>aiScene.mLights</code>).
       *
       * The corresponding scenegraph nodes are <em>not</em> removed. Use the
       * <code>OptimizeGraph</code> step to do this.
       */
      LIGHTS = 0x100,

      /**
       * Cameras (<code>aiScene.mCameras</code>).
       *
       * The corresponding scenegraph nodes are <em>not</em> removed. Use the
       * <code>OptimizeGraph</code> step to do this.
       */
      CAMERAS = 0x200,

      /**
       * Meshes (<code>aiScene.mMeshes</code>).
       */
      MESHES = 0x400,

      /** Materials.
       *
       * One default material will be generated, so
       * <code>aiScene.mNumMaterials</code> will be 1.
      */
      MATERIALS = 0x800
   }

   /**
    * Specifies a certain color channel to remove.
    */
   uint aiComponent_COLORSn( uint n ) { return 1u << ( n + 20u ); }

   /**
    * Specifies a certain UV coordinate channel to remove.
    */
   uint aiComponent_TEXCOORDSn( uint n ) { return 1u << ( n + 25u ); }

   /**
    * Input parameter to the <code>RemoveComponent</code> step:
    * Specifies the parts of the data structure to be removed.
    *
    * See the documentation to this step for further details.
    *
    * Default value: 0
    *
    * Property type: integer (bitwise combination of <code>aiComponent</code>
    * flags).
    *
    * Note: If no valid mesh is remaining after the step has been executed, the
    *    import <em>fails</em>, because there is no data to work on anymore.
    */
   const char* AI_CONFIG_PP_RVC_FLAGS = "PP_RVC_FLAGS";

   /**
    * Input parameter to the <code>SortByPType</code> step:
    * Specifies which primitive types are removed by the step.
    *
    * This is a bitwise combination of the <code>aiPrimitiveType</code> flags.
    * Specifying all of them is illegal, of course. A typical use would be to
    * exclude all line and point meshes from the import.
    *
    * Default value: 0
    *
    * Property type: integer.
    */
   const char* AI_CONFIG_PP_SBP_REMOVE = "PP_SBP_REMOVE";

   /**
    * Input parameter to the <code>FindInvalidData</code> step:
    * Specifies the floating-point accuracy for animation values.
    *
    * The step checks for animation tracks where all frame values are
    * absolutely equal and removes them. This tweakable controls the epsilon
    * for floating-point comparisons – two keys are considered equal if the
    * invariant abs(n0-n1) > epsilon holds true for all vector respectively
    * quaternion components.
    *
    * Default value: 0 (exact comparison).
    *
    * Property type: float.
    */
   const char* AI_CONFIG_PP_FID_ANIM_ACCURACY = "PP_FID_ANIM_ACCURACY";

   /**
    * The <code>TransformUVCoords</code> step evaluates UV scalings.
    */
   const AI_UVTRAFO_SCALING = 0x1;

   /**
    * The <code>TransformUVCoords</code> step evaluates UV rotations.
    */
   const AI_UVTRAFO_ROTATION = 0x2;

   /**
    * The <code>TransformUVCoords</code> step evaluates UV translation.
    */
   const AI_UVTRAFO_TRANSLATION = 0x4;

   /**
    * The <code>TransformUVCoords</code> step evaluates all UV translations.
    */
   const AI_UVTRAFO_ALL =
      AI_UVTRAFO_SCALING
      | AI_UVTRAFO_ROTATION
      | AI_UVTRAFO_TRANSLATION;

   /**
    * Input parameter to the <code>TransformUVCoords</code> step: Specifies
    * which UV transformations are evaluated.
    *
    * Default value: AI_UVTRAFO_ALL.
    *
    * Property type: integer (bitwise combination of the
    * <code>AI_UVTRAFO_XXX<code> flag).
    */
   const char* AI_CONFIG_PP_TUV_EVALUATE = "PP_TUV_EVALUATE";

   /**
    * A hint to assimp to favour speed against import quality.
    *
    * Enabling this option may result in faster loading, but it needn't.
    * It represents just a hint to loaders and post-processing steps to use
    * faster code paths, if possible.
    *
    * Default value: false.
    *
    * Property type: bool.
    */
   const char* AI_CONFIG_FAVOUR_SPEED = "FAVOUR_SPEED";


   /*
    * Importer settings.
    *
    * Various stuff to fine-tune the behaviour of specific importer plugins.
    */

   /**
    * Set the vertex animation keyframe to be imported.
    *
    * Assimp does not support vertex keyframes (only bone animation is
    * supported). The library reads only one frame of models with vertex
    * animations.
    *
    * Default value: 0 (first frame).
    *
    * Property type: integer.
    *
    * Note: This option applies to all importers. However, it is also possible
    *    to override the global setting for a specific loader. You can use the
    *    AI_CONFIG_IMPORT_XXX_KEYFRAME options (where XXX is a placeholder for
    *    the file format for which you want to override the global setting).
    */
   const char* AI_CONFIG_IMPORT_GLOBAL_KEYFRAME = "IMPORT_GLOBAL_KEYFRAME";

   const char* AI_CONFIG_IMPORT_MD3_KEYFRAME = "IMPORT_MD3_KEYFRAME";
   const char* AI_CONFIG_IMPORT_MD2_KEYFRAME = "IMPORT_MD2_KEYFRAME";
   const char* AI_CONFIG_IMPORT_MDL_KEYFRAME = "IMPORT_MDL_KEYFRAME";
   const char* AI_CONFIG_IMPORT_MDC_KEYFRAME = "IMPORT_MDC_KEYFRAME";
   const char* AI_CONFIG_IMPORT_SMD_KEYFRAME = "IMPORT_SMD_KEYFRAME";
   const char* AI_CONFIG_IMPORT_UNREAL_KEYFRAME = "IMPORT_UNREAL_KEYFRAME";


   /**
    * Configures the AC loader to collect all surfaces which have the
    * "Backface cull" flag set in separate meshes.
    *
    * Default value: true.
    *
    * Property type: bool.
    */
   const char* AI_CONFIG_IMPORT_AC_SEPARATE_BFCULL = "IMPORT_AC_SEPARATE_BFCULL";

   /**
    * Configures the UNREAL 3D loader to separate faces with different surface
    * flags (e.g. two-sided vs. single-sided).
    *
    * Default value: true.
    *
    * Property type: bool.
    */
   const char* AI_CONFIG_IMPORT_UNREAL_HANDLE_FLAGS = "UNREAL_HANDLE_FLAGS";

   /**
    * Configures the terragen import plugin to compute uv's for terrains, if
    * not given. Furthermore, a default texture is assigned.
    *
    * UV coordinates for terrains are so simple to compute that you'll usually
    * want to compute them on your own, if you need them. This option is intended
    * for model viewers which want to offer an easy way to apply textures to
    * terrains.
    *
    * Default value: false.
    *
    * Property type: bool.
    */
   const char* AI_CONFIG_IMPORT_TER_MAKE_UVS = "IMPORT_TER_MAKE_UVS";

   /**
    * Configures the ASE loader to always reconstruct normal vectors basing on
    * the smoothing groups loaded from the file.
    *
    * Many ASE files have invalid normals (they're not orthonormal).
    *
    * Default value: true.
    *
    * Property type: bool.
    */
   const char* AI_CONFIG_IMPORT_ASE_RECONSTRUCT_NORMALS = "IMPORT_ASE_RECONSTRUCT_NORMALS";

   /**
    * Configures the M3D loader to detect and process multi-part Quake player
    * models.
    *
    * These models usually consist of three files, <code>lower.md3</code>,
    * <code>upper.md3</code> and <code>head.md3</code>. If this property is set
    * to true, Assimp will try to load and combine all three files if one of
    * them is loaded.
    *
    * Default value: true.
    *
    * Property type: bool.
    */
   const char* AI_CONFIG_IMPORT_MD3_HANDLE_MULTIPART = "IMPORT_MD3_HANDLE_MULTIPART";

   /**
    * Tells the MD3 loader which skin files to load.
    *
    * When loading MD3 files, Assimp checks whether a file
    * <code><md3_file_name>_<skin_name>.skin</code> is existing. These files
    * are used by Quake 3 to be able to assign different skins (e.g. red and
    * blue team) to models. 'default', 'red', 'blue' are typical skin names.
    *
    * Default value: "default".
    *
    * Property type: string.
    */
   const char* AI_CONFIG_IMPORT_MD3_SKIN_NAME = "IMPORT_MD3_SKIN_NAME";

   /**
    * Specify the Quake 3 shader file to be used for a particular MD3 file.
    * This can also be a search path.
    *
    * By default Assimp's behaviour is as follows: If a MD3 file
    * <code>[any_path]/models/[any_q3_subdir]/[model_name]/[file_name].md3</code>
    * is loaded, the library tries to locate the corresponding shader file in
    * <code>[any_path]/scripts/[model_name].shader</code>. This property
    * overrides this behaviour. It can either specify a full path to the shader
    * to be loaded or alternatively the path (relative or absolute) to the
    * directory where the shaders for all MD3s to be loaded reside. Assimp
    * attempts to open <code>[dir]/[model_name].shader</code> first,
    * <code>[dir]/[file_name].shader</code> is the fallback file. Note that
    * <code>[dir]</code> should have a terminal (back)slash.
    *
    * Default value: n/a.
    *
    * Property type: string.
    */
   const char* AI_CONFIG_IMPORT_MD3_SHADER_SRC = "IMPORT_MD3_SHADER_SRC";

   /**
    * Configures the LWO loader to load just one layer from the model.
    *
    * LWO files consist of layers and in some cases it could be useful to load
    * only one of them. This property can be either a string – which specifies
    * the name of the layer – or an integer – the index of the layer. If the
    * property is not set the whole LWO model is loaded. Loading fails if the
    * requested layer is not available. The layer index is zero-based and the
    * layer name may not be empty.
    *
    * Default value: all layers are loaded.
    *
    * Property type: integer/string.
    */
   const char* AI_CONFIG_IMPORT_LWO_ONE_LAYER_ONLY = "IMPORT_LWO_ONE_LAYER_ONLY";

   /**
    * Configures the MD5 loader to not load the MD5ANIM file for a MD5MESH file
    * automatically.
    *
    * The default strategy is to look for a file with the same name but the
    * MD5ANIM extension in the same directory. If it is found, it is loaded
    * and combined with the MD5MESH file. This configuration option can be
    * used to disable this behaviour.
    *
    * Default value: false.
    *
    * Property type: bool.
    */
   const char* AI_CONFIG_IMPORT_MD5_NO_ANIM_AUTOLOAD = "IMPORT_MD5_NO_ANIM_AUTOLOAD";

   /**
    * Defines the begin of the time range for which the LWS loader evaluates
    * animations and computes <code>aiNodeAnim</code>s.
    *
    * Assimp provides full conversion of LightWave's envelope system, including
    * pre and post conditions. The loader computes linearly subsampled animation
    * chanels with the frame rate given in the LWS file. This property defines
    * the start time. Note: animation channels are only generated if a node
    * has at least one envelope with more tan one key assigned. This property.
    * is given in frames, '0' is the first frame. By default, if this property
    * is not set, the importer takes the animation start from the input LWS
    * file ('FirstFrame' line).
    *
    * Default value: read from file.
    *
    * Property type: integer.
    *
    * See: <code>AI_CONFIG_IMPORT_LWS_ANIM_END</code> – end of the imported
    *    time range
    */
   const char* AI_CONFIG_IMPORT_LWS_ANIM_START = "IMPORT_LWS_ANIM_START";
   const char* AI_CONFIG_IMPORT_LWS_ANIM_END = "IMPORT_LWS_ANIM_END";

   /**
    * Defines the output frame rate of the IRR loader.
    *
    * IRR animations are difficult to convert for Assimp and there will always
    * be a loss of quality. This setting defines how many keys per second are
    * returned by the converter.
    *
    * Default value: 100.
    *
    * Property type: integer.
    */
   const char* AI_CONFIG_IMPORT_IRR_ANIM_FPS = "IMPORT_IRR_ANIM_FPS";

   /**
    * Ogre Importer will try to load this material file.
    *
    * Ogre Mehs contain only the material name, not the material file. If there
    * is no material file with the same name as the material, Ogre Importer
    * will try to load this file and search the material in it.
    *
    * Property type: string. Default value: "Scene.material".
    */
   const char* AI_CONFIG_IMPORT_OGRE_MATERIAL_FILE = "IMPORT_OGRE_MATERIAL_FILE";
}
