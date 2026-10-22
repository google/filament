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
 * Definitions for import post processing steps.
 */
module assimp.postprocess;

extern ( C ) {
   /**
    * Defines the flags for all possible post processing steps.
    *
    * See: <code>aiImportFile</code>, <code>aiImportFileEx</code>
    */
   enum aiPostProcessSteps {
      /**
       * Calculates the tangents and bitangents for the imported meshes.
       *
       * Does nothing if a mesh does not have normals. You might want this post
       * processing step to be executed if you plan to use tangent space
       * calculations such as normal mapping applied to the meshes. There is a
       * config setting, <code>AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE</code>,
       * which allows you to specify a maximum smoothing angle for the
       * algorithm. However, usually you will want to use the default value.
       */
      CalcTangentSpace = 0x1,

      /**
       * Identifies and joins identical vertex data sets within all imported
       * meshes.
       *
       * After this step is run each mesh does contain only unique vertices
       * anymore, so a vertex is possibly used by multiple faces. You usually
       * want to use this post processing step. If your application deals with
       * indexed geometry, this step is compulsory or you will just waste
       * rendering time. <em>If this flag is not specified</em>, no vertices
       * are referenced by more than one face and <em>no index buffer is
       * required</em> for rendering.
       */
      JoinIdenticalVertices = 0x2,

      /**
       * Converts all the imported data to a left-handed coordinate space.
       *
       * By default the data is returned in a right-handed coordinate space
       * which for example OpenGL prefers. In this space, +X points to the
       * right, +Z points towards the viewer and and +Y points upwards. In the
       * DirectX coordinate space +X points to the right, +Y points upwards and
       * +Z points away from the viewer.
       *
       * You will probably want to consider this flag if you use Direct3D for
       * rendering. The <code>ConvertToLeftHanded</code> flag supersedes this
       * setting and bundles all conversions typically required for D3D-based
       * applications.
       */
      MakeLeftHanded = 0x4,

      /**
       * Triangulates all faces of all meshes.
       *
       * By default the imported mesh data might contain faces with more than 3
       * indices. For rendering you'll usually want all faces to be triangles.
       * This post processing step splits up all higher faces to triangles.
       * Line and point primitives are <em>not</em> modified!.
       *
       * If you want »triangles only« with no other kinds of primitives,
       * specify both <code>Triangulate</code> and <code>SortByPType</code> and
       * ignore all point and line meshes when you process Assimp's output.
       */
      Triangulate = 0x8,

      /**
       * Removes some parts of the data structure (animations, materials, light
       * sources, cameras, textures, vertex components).
       *
       * The components to be removed are specified in a separate configuration
       * option, <code>AI_CONFIG_PP_RVC_FLAGS</code>. This is quite useful if
       * you don't need all parts of the output structure. Especially vertex
       * colors are rarely used today.
       *
       * Calling this step to remove unrequired stuff from the pipeline as
       * early as possible results in an increased performance and a better
       * optimized output data structure.
       *
       * This step is also useful if you want to force Assimp to recompute
       * normals or tangents since the corresponding steps don't recompute them
       * if they have already been loaded from the source asset.
       *
       * This flag is a poor one, mainly because its purpose is usually
       * misunderstood. Consider the following case: a 3d model has been exported
       * from a CAD app, it has per-face vertex colors. Because of the vertex
       * colors (which are not even used by most apps),
       * <code>JoinIdenticalVertices</code> cannot join vertices at the same
       * position. By using this step, unneeded components are excluded as
       * early as possible thus opening more room for internal optimzations.
       */
      RemoveComponent = 0x10,

      /**
       * Generates normals for all faces of all meshes.
       *
       * This is ignored if normals are already there at the time where this
       * flag is evaluated. Model importers try to load them from the source
       * file, so they are usually already there. Face normals are shared
       * between all points of a single face, so a single point can have
       * multiple normals, which, in other words, enforces the library to
       * duplicate vertices in some cases. <code>JoinIdenticalVertices</code>
       * is <em>useless</em> then.
       *
       * This flag may not be specified together with
       * <code>GenSmoothNormals</code>.
       */
      GenNormals = 0x20,

      /**
       * Generates smooth normals for all vertices in the mesh.
       *
       * This is ignored if normals are already there at the time where this
       * flag is evaluated. Model importers try to load them from the source file, so
       * they are usually already there.
       *
       * There is a configuration option,
       * <code>AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE</code> which allows you to
       * specify an angle maximum for the normal smoothing algorithm. Normals
       * exceeding this limit are not smoothed, resulting in a »hard« seam
       * between two faces. Using a decent angle here (e.g. 80°) results in
       * very good visual appearance.
       */
      GenSmoothNormals = 0x40,

      /**
       * Splits large meshes into smaller submeshes.
       *
       * This is quite useful for realtime rendering where the number of triangles
       * which can be maximally processed in a single draw-call is usually limited
       * by the video driver/hardware. The maximum vertex buffer is usually limited,
       * too. Both requirements can be met with this step: you may specify both a
       * triangle and vertex limit for a single mesh.
       *
       * The split limits can (and should!) be set through the
       * <code>AI_CONFIG_PP_SLM_VERTEX_LIMIT</code> and
       * <code>AI_CONFIG_PP_SLM_TRIANGLE_LIMIT</code> settings. The default
       * values are <code>AI_SLM_DEFAULT_MAX_VERTICES</code> and
       * <code>AI_SLM_DEFAULT_MAX_TRIANGLES</code>.
       *
       * Note that splitting is generally a time-consuming task, but not if
       * there's nothing to split. The use of this step is recommended for most
       * users.
       */
      SplitLargeMeshes = 0x80,

      /**
       * Removes the node graph and pre-transforms all vertices with the local
       * transformation matrices of their nodes.
       *
       * The output scene does still contain nodes, however, there is only a
       * root node with children, each one referencing only one mesh, each
       * mesh referencing one material. For rendering, you can simply render
       * all meshes in order, you don't need to pay attention to local
       * transformations and the node hierarchy. Animations are removed during
       * this step.
       *
       * This step is intended for applications that have no scenegraph.
       *
       * The step <em>can</em> cause some problems: if e.g. a mesh of the asset
       * contains normals and another, using the same material index, does not,
       * they will be brought together, but the first meshes's part of the
       * normal list is zeroed. However, these artifacts are rare.
       *
       * Note: The <code>AI_CONFIG_PP_PTV_NORMALIZE</code> configuration
       *    property can be set to normalize the scene's spatial dimension
       *    to the -1...1 range.
       */
      PreTransformVertices = 0x100,

      /**
       * Limits the number of bones simultaneously affecting a single vertex to
       * a maximum value.
       *
       * If any vertex is affected by more than that number of bones, the least
       * important vertex weights are removed and the remaining vertex weights
       * are renormalized so that the weights still sum up to 1.
       *
       * The default bone weight limit is 4 (<code>AI_LMW_MAX_WEIGHTS</code>),
       * but you can use the <code>#AI_CONFIG_PP_LBW_MAX_WEIGHTS</code> setting
       * to supply your own limit to the post processing step.
       *
       * If you intend to perform the skinning in hardware, this post processing
       * step might be of interest for you.
       */
      LimitBoneWeights = 0x200,

      /**
       * Validates the imported scene data structure.
       *
       * This makes sure that all indices are valid, all animations and
       * bones are linked correctly, all material references are correct, etc.
       *
       * It is recommended to capture Assimp's log output if you use this flag,
       * so you can easily find ot what's actually wrong if a file fails the
       * validation. The validator is quite rude and will find <em>all</em>
       * inconsistencies in the data structure.
       *
       * Plugin developers are recommended to use it to debug their loaders.
       *
       * There are two types of validation failures:
       * <ul>
       * <li>Error: There's something wrong with the imported data. Further
       *    postprocessing is not possible and the data is not usable at all.
       *    The import fails, see <code>aiGetErrorString()</code> for the
       *    error message.</li>
       * <li>Warning: There are some minor issues (e.g. 1000000 animation
       *   keyframes with the same time), but further postprocessing and use
       *   of the data structure is still safe. Warning details are written
       *   to the log file, <code>AI_SCENE_FLAGS_VALIDATION_WARNING</code> is
       *   set in <code>aiScene::mFlags</code></li>
       * </ul>
       *
       * This post-processing step is not time-consuming. It's use is not
       * compulsory, but recommended.
       */
      ValidateDataStructure = 0x400,

      /**
       * Reorders triangles for better vertex cache locality.
       *
       * The step tries to improve the ACMR (average post-transform vertex cache
       * miss ratio) for all meshes. The implementation runs in O(n) and is
       * roughly based on the 'tipsify' algorithm (see
       * <tt>http://www.cs.princeton.edu/gfx/pubs/Sander_2007_%3ETR/tipsy.pdf</tt>).
       *
       * If you intend to render huge models in hardware, this step might
       * be of interest for you. The <code>AI_CONFIG_PP_ICL_PTCACHE_SIZE</code>
       * config setting can be used to fine-tune the cache optimization.
       */
      ImproveCacheLocality = 0x800,

      /**
       * Searches for redundant/unreferenced materials and removes them.
       *
       * This is especially useful in combination with the
       * <code>PretransformVertices</code> and <code>OptimizeMeshes</code>
       * flags. Both join small meshes with equal characteristics, but they
       * can't do their work if two meshes have different materials. Because
       * several material settings are always lost during Assimp's import
       * filters, (and because many exporters don't check for redundant
       * materials), huge models often have materials which are are defined
       * several times with exactly the same settings.
       *
       * Several material settings not contributing to the final appearance of
       * a surface are ignored in all comparisons; the material name is one of
       * them. So, if you are passing additional information through the
       * content pipeline (probably using »magic« material names), don't
       * specify this flag. Alternatively take a look at the
       * <code>AI_CONFIG_PP_RRM_EXCLUDE_LIST</code> setting.
       */
      RemoveRedundantMaterials = 0x1000,

      /**
       * This step tries to determine which meshes have normal vectors that are
       * acing inwards.
       *
       * The algorithm is simple but effective: The bounding box of all
       * vertices and their normals is compared against the volume of the
       * bounding box of all vertices without their normals. This works well
       * for most objects, problems might occur with planar surfaces. However,
       * the step tries to filter such cases.
       *
       * The step inverts all in-facing normals. Generally it is recommended to
       * enable this step, although the result is not always correct.
       */
      FixInfacingNormals = 0x2000,

      /**
       * This step splits meshes with more than one primitive type in
       * homogeneous submeshes.
       *
       * The step is executed after the triangulation step. After the step
       * returns, just one bit is set in <code>aiMesh.mPrimitiveTypes</code>.
       * This is especially useful for real-time rendering where point and line
       * primitives are often ignored or rendered separately.
       *
       * You can use the <code>AI_CONFIG_PP_SBP_REMOVE</code> option to
       * specify which primitive types you need. This can be used to easily
       * exclude lines and points, which are rarely used, from the import.
       */
      SortByPType = 0x8000,

      /**
       * This step searches all meshes for degenerated primitives and converts
       * them to proper lines or points.
       *
       * A face is »degenerated« if one or more of its points are identical.
       * To have the degenerated stuff not only detected and collapsed but also
       * removed, try one of the following procedures:
       *
       * <b>1.</b> (if you support lines and points for rendering but don't
       *    want the degenerates)
       * <ul>
       *   <li>Specify the <code>FindDegenerates</code> flag.</li>
       *   <li>Set the <code>AI_CONFIG_PP_FD_REMOVE</code> option to 1. This will
       *       cause the step to remove degenerated triangles from the import
       *       as soon as they're detected. They won't pass any further
       *       pipeline steps.</li>
       * </ul>
       *
       * <b>2.</b>(if you don't support lines and points at all ...)
       * <ul>
       *   <li>Specify the <code>FindDegenerates</code> flag.</li>
       *   <li>Specify the <code>SortByPType</code> flag. This moves line and
       *      point primitives to separate meshes.</li>
       *   <li>Set the <code>AI_CONFIG_PP_SBP_REMOVE</codet> option to
       *      <code>aiPrimitiveType_POINTS | aiPrimitiveType_LINES</code>
       *      to cause SortByPType to reject point and line meshes from the
       *      scene.</li>
       * </ul>
       *
       * Note: Degenerated polygons are not necessarily bad and that's why
       *    they're not removed by default. There are several file formats
       *    which don't support lines or points. Some exporters bypass the
       *    format specification and write them as degenerated triangle
       *    instead.
       */
      FindDegenerates = 0x10000,

      /**
       * This step searches all meshes for invalid data, such as zeroed normal
       * vectors or invalid UV coords and removes/fixes them. This is intended
       * to get rid of some common exporter errors.
       *
       * This is especially useful for normals. If they are invalid, and the
       * step recognizes this, they will be removed and can later be
       * recomputed, e.g. by the <code>GenSmoothNormals</code> step.
       *
       * The step will also remove meshes that are infinitely small and reduce
       * animation tracks consisting of hundreds if redundant keys to a single
       * key. The <code>AI_CONFIG_PP_FID_ANIM_ACCURACY</code> config property
       * decides the accuracy of the check for duplicate animation tracks.
       */
      FindInvalidData = 0x20000,

      /**
       * This step converts non-UV mappings (such as spherical or cylindrical
       * mapping) to proper texture coordinate channels.
       *
       * Most applications will support UV mapping only, so you will probably
       * want to specify this step in every case. Note tha Assimp is not always
       * able to match the original mapping implementation of the 3d app which
       * produced a model perfectly. It's always better to let the father app
       * compute the UV channels, at least 3ds max, maja, blender, lightwave,
       * modo, ... are able to achieve this.
       *
       * Note: If this step is not requested, you'll need to process the
       *    <code>AI_MATKEY_MAPPING</code> material property in order to
       *    display all assets properly.
       */
      GenUVCoords = 0x40000,

      /**
       * This step applies per-texture UV transformations and bakes them to
       * stand-alone vtexture coordinate channelss.
       *
       * UV transformations are specified per-texture – see the
       * <code>AI_MATKEY_UVTRANSFORM</code> material key for more information.
       * This step processes all textures with transformed input UV coordinates
       * and generates new (pretransformed) UV channel which replace the old
       * channel. Most applications won't support UV transformations, so you
       * will probably want to specify this step.
       *
       * Note: UV transformations are usually implemented in realtime apps by
       *    transforming texture coordinates at vertex shader stage with a 3x3
       *    (homogenous) transformation matrix.
       */
      TransformUVCoords = 0x80000,

      /**
       * This step searches for duplicate meshes and replaces duplicates with
       * references to the first mesh.
       *
       * This step takes a while, don't use it if you have no time. Its main
       * purpose is to workaround the limitation that many export file formats
       * don't support instanced meshes, so exporters need to duplicate meshes.
       * This step removes the duplicates again. Please note that Assimp does
       * currently not support per-node material assignment to meshes, which
       * means that identical meshes with differnent materials are currently
       * <em>not</em> joined, although this is planned for future versions.
       */
      FindInstances = 0x100000,

      /**
       * A postprocessing step to reduce the number of meshes.
       *
       * In fact, it will reduce the number of drawcalls.
       *
       * This is a very effective optimization and is recommended to be used
       * together with <code>OptimizeGraph</code>, if possible. The flag is
       * fully compatible with both <code>SplitLargeMeshes</code> and
       * <code>SortByPType</code>.
       */
      OptimizeMeshes = 0x200000,

      /**
       * A postprocessing step to optimize the scene hierarchy.
       *
       * Nodes with no animations, bones, lights or cameras assigned are
       * collapsed and joined.
       *
       * Node names can be lost during this step. If you use special tag nodes
       * to pass additional information through your content pipeline, use the
       * <code>AI_CONFIG_PP_OG_EXCLUDE_LIST</code> setting to specify a list of
       * node names you want to be kept. Nodes matching one of the names in
       * this list won't be touched or modified.
       *
       * Use this flag with caution. Most simple files will be collapsed to a
       * single node, complex hierarchies are usually completely lost. That's
       * note the right choice for editor environments, but probably a very
       * effective optimization if you just want to get the model data, convert
       * it to your own format and render it as fast as possible.
       *
       * This flag is designed to be used with <code>OptimizeMeshes</code> for
       * best results.
       *
       * Note: »Crappy« scenes with thousands of extremely small meshes packed
       *    in deeply nested nodes exist for almost all file formats.
       *    <code>OptimizeMeshes</code> in combination with
       *    <code>OptimizeGraph</code> usually fixes them all and makes them
       *    renderable.
       */
      OptimizeGraph  = 0x400000,

      /** This step flips all UV coordinates along the y-axis and adjusts
       * material settings and bitangents accordingly.
       *
       * Output UV coordinate system:
       * <pre> 0y|0y ---------- 1x|0y
       * |                 |
       * |                 |
       * |                 |
       * 0x|1y ---------- 1x|1y</pre>
       * You'll probably want to consider this flag if you use Direct3D for
       * rendering. The <code>AI_PROCESS_CONVERT_TO_LEFT_HANDED</code> flag
       * supersedes this setting and bundles all conversions typically required
       * for D3D-based applications.
      */
      FlipUVs = 0x800000,

      /**
       * This step adjusts the output face winding order to be clockwise.
       *
       * The default face winding order is counter clockwise.
       *
       * Output face order:
       * <pre>       x2
       *
       *                         x0
       *  x1</pre>
       */
      FlipWindingOrder  = 0x1000000
   }

   /**
    * Abbrevation for convenience.
    */
   alias aiPostProcessSteps aiProcess;

   /**
    * Shortcut flag for Direct3D-based applications.
    *
    * Combines the <code>MakeLeftHanded</code>, <code>FlipUVs</code> and
    * <code>FlipWindingOrder</code> flags. The output data matches Direct3D's
    * conventions: left-handed geometry, upper-left origin for UV coordinates
    * and clockwise face order, suitable for CCW culling.
    */
   const aiPostProcessSteps AI_PROCESS_CONVERT_TO_LEFT_HANDED =
      aiProcess.MakeLeftHanded |
      aiProcess.FlipUVs |
      aiProcess.FlipWindingOrder;

   /**
    * Default postprocess configuration optimizing the data for real-time rendering.
    *
    * Applications would want to use this preset to load models on end-user
    * PCs, maybe for direct use in game.
    *
    * If you're using DirectX, don't forget to combine this value with
    * the <code>ConvertToLeftHanded</code> step. If you don't support UV
    * transformations in your application, apply the
    * <code>TransformUVCoords</code> step, too.
    *
    * Note: Please take the time to read the doc for the steps enabled by this
    *   preset. Some of them offer further configurable properties, some of
    *   them might not be of use for you so it might be better to not specify
    *   them.
    */
   const aiPostProcessSteps AI_PROCESS_PRESET_TARGET_REALTIME_FAST =
      aiProcess.CalcTangentSpace |
      aiProcess.GenNormals |
      aiProcess.JoinIdenticalVertices |
      aiProcess.Triangulate |
      aiProcess.GenUVCoords |
      aiProcess.SortByPType;

    /**
     * Default postprocess configuration optimizing the data for real-time
     * rendering.
     *
     * Unlike <code>AI_PROCESS_PRESET_TARGET_REALTIME_FAST</code>, this
     * configuration performs some extra optimizations to improve rendering
     * speed and to minimize memory usage. It could be a good choice for a
     * level editor environment where import speed is not so important.
     *
     * If you're using DirectX, don't forget to combine this value with
     * the <code>ConvertToLeftHanded</code> step. If you don't support UV
     * transformations in your application, apply the
     * <code>TransformUVCoords</code> step, too.
     *
     * Note: Please take the time to read the doc for the steps enabled by this
     *   preset. Some of them offer further configurable properties, some of
     *   them might not be of use for you so it might be better to not specify
     *   them.
     */
   const aiPostProcessSteps AI_PROCESS_PRESET_TARGET_REALTIME_QUALITY =
      aiProcess.CalcTangentSpace |
      aiProcess.GenSmoothNormals |
      aiProcess.JoinIdenticalVertices |
      aiProcess.ImproveCacheLocality |
      aiProcess.LimitBoneWeights |
      aiProcess.RemoveRedundantMaterials |
      aiProcess.SplitLargeMeshes |
      aiProcess.Triangulate |
      aiProcess.GenUVCoords |
      aiProcess.SortByPType |
      aiProcess.FindDegenerates |
      aiProcess.FindInvalidData;

    /**
     * Default postprocess configuration optimizing the data for real-time
     * rendering.
     *
     * This preset enables almost every optimization step to achieve perfectly
     * optimized data. It's your choice for level editor environments where
     * import speed is not important.
     *
     * If you're using DirectX, don't forget to combine this value with
     * the <code>ConvertToLeftHanded</code> step. If you don't support UV
     * transformations in your application, apply the
     * <code>TransformUVCoords</code> step, too.
     *
     * Note: Please take the time to read the doc for the steps enabled by this
     *   preset. Some of them offer further configurable properties, some of
     *   them might not be of use for you so it might be better to not specify
     *   them.
     */
   const aiPostProcessSteps AI_PROCESS_PRESET_TARGET_REALTIME_MAX_QUALITY =
      AI_PROCESS_PRESET_TARGET_REALTIME_QUALITY |
      aiProcess.FindInstances |
      aiProcess.ValidateDataStructure |
      aiProcess.OptimizeMeshes;
}
