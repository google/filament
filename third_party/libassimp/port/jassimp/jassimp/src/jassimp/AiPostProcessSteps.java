/*
---------------------------------------------------------------------------
Open Asset Import Library - Java Binding (jassimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2012, assimp team

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
package jassimp;

import java.util.Set;

/**
 * Enumerates the post processing steps supported by assimp.
 */
public enum AiPostProcessSteps {

    /**
     * Calculates the tangents and bitangents for the imported meshes.
     * <p>
     * 
     * Does nothing if a mesh does not have normals. You might want this post
     * processing step to be executed if you plan to use tangent space
     * calculations such as normal mapping applied to the meshes. There's a
     * config setting, <tt>#AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE</tt>, which
     * allows you to specify a maximum smoothing angle for the algorithm.
     * However, usually you'll want to leave it at the default value.
     */
    CALC_TANGENT_SPACE(0x1),


    /**
     * Identifies and joins identical vertex data sets within all imported
     * meshes.<p>
     * 
     * After this step is run, each mesh contains unique vertices, so a vertex
     * may be used by multiple faces. You usually want to use this post
     * processing step. If your application deals with indexed geometry, this
     * step is compulsory or you'll just waste rendering time. <b>If this flag
     * is not specified</b>, no vertices are referenced by more than one face
     * and <b>no index buffer is required</b> for rendering.
     */
    JOIN_IDENTICAL_VERTICES(0x2),


    /**
     * Converts all the imported data to a left-handed coordinate space.<p>
     * 
     * By default the data is returned in a right-handed coordinate space (which
     * OpenGL prefers). In this space, +X points to the right, +Z points towards
     * the viewer, and +Y points upwards. In the DirectX coordinate space +X
     * points to the right, +Y points upwards, and +Z points away from the
     * viewer.<p>
     * 
     * You'll probably want to consider this flag if you use Direct3D for
     * rendering. The #ConvertToLeftHanded flag supersedes this
     * setting and bundles all conversions typically required for D3D-based
     * applications.
     */
    MAKE_LEFT_HANDED(0x4),


    /**
     * Triangulates all faces of all meshes.<p>
     * 
     * By default the imported mesh data might contain faces with more than 3
     * indices. For rendering you'll usually want all faces to be triangles.
     * This post processing step splits up faces with more than 3 indices into
     * triangles. Line and point primitives are *not* modified! If you want
     * 'triangles only' with no other kinds of primitives, try the following
     * solution:
     * <ul>
     *   <li>Specify both #Triangulate and #SortByPType
     *   <li>Ignore all point and line meshes when you process assimp's output
     * </ul>
     */
    TRIANGULATE(0x8),


    /**
     * Removes some parts of the data structure (animations, materials, light
     * sources, cameras, textures, vertex components).<p>
     * 
     * The components to be removed are specified in a separate configuration
     * option, <tt>#AI_CONFIG_PP_RVC_FLAGS</tt>. This is quite useful if you
     * don't need all parts of the output structure. Vertex colors are rarely
     * used today for example... Calling this step to remove unneeded data from
     * the pipeline as early as possible results in increased performance and a
     * more optimized output data structure. This step is also useful if you
     * want to force Assimp to recompute normals or tangents. The corresponding
     * steps don't recompute them if they're already there (loaded from the
     * source asset). By using this step you can make sure they are NOT there.
     * <p>
     * 
     * This flag is a poor one, mainly because its purpose is usually
     * misunderstood. Consider the following case: a 3D model has been exported
     * from a CAD app, and it has per-face vertex colors. Vertex positions can't
     * be shared, thus the #JoinIdenticalVertices step fails to
     * optimize the data because of these nasty little vertex colors. Most apps
     * don't even process them, so it's all for nothing. By using this step,
     * unneeded components are excluded as early as possible thus opening more
     * room for internal optimizations.
     */
    REMOVE_COMPONENT(0x10),


    /**
     * Generates normals for all faces of all meshes.<p>
     * 
     * This is ignored if normals are already there at the time this flag is
     * evaluated. Model importers try to load them from the source file, so
     * they're usually already there. Face normals are shared between all points
     * of a single face, so a single point can have multiple normals, which
     * forces the library to duplicate vertices in some cases.
     * #JoinIdenticalVertices is *senseless* then.<p>
     * 
     * This flag may not be specified together with {@link #GEN_SMOOTH_NORMALS}.
     */
    GEN_NORMALS(0x20),


    /**
     * Generates smooth normals for all vertices in the mesh.<p>
     * 
     * This is ignored if normals are already there at the time this flag is
     * evaluated. Model importers try to load them from the source file, so
     * they're usually already there.<p>
     * 
     * This flag may not be specified together with {@link #GEN_NORMALS}
     * There's a configuration option,
     * <tt>#AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE</tt> which allows you to
     * specify an angle maximum for the normal smoothing algorithm. Normals
     * exceeding this limit are not smoothed, resulting in a 'hard' seam between
     * two faces. Using a decent angle here (e.g. 80 degrees) results in very
     * good visual appearance.
     */
    GEN_SMOOTH_NORMALS(0x40),


    /**
     * Splits large meshes into smaller sub-meshes.<p>
     * 
     * This is quite useful for real-time rendering, where the number of
     * triangles which can be maximally processed in a single draw-call is
     * limited by the video driver/hardware. The maximum vertex buffer is
     * usually limited too. Both requirements can be met with this step: you may
     * specify both a triangle and vertex limit for a single mesh.<p>
     * 
     * The split limits can (and should!) be set through the
     * <tt>#AI_CONFIG_PP_SLM_VERTEX_LIMIT</tt> and
     * <tt>#AI_CONFIG_PP_SLM_TRIANGLE_LIMIT</tt> settings. The default values
     * are <tt>#AI_SLM_DEFAULT_MAX_VERTICES</tt> and
     * <tt>#AI_SLM_DEFAULT_MAX_TRIANGLES</tt>.<p>
     * 
     * Note that splitting is generally a time-consuming task, but only if
     * there's something to split. The use of this step is recommended for most
     * users.
     */
    SPLIT_LARGE_MESHES(0x80),


    /**
     * Removes the node graph and pre-transforms all vertices with the local
     * transformation matrices of their nodes.<p>
     * 
     * The output scene still contains nodes, however there is only a root node
     * with children, each one referencing only one mesh, and each mesh
     * referencing one material. For rendering, you can simply render all meshes
     * in order - you don't need to pay attention to local transformations and
     * the node hierarchy. Animations are removed during this step. This step is
     * intended for applications without a scenegraph. The step CAN cause some
     * problems: if e.g. a mesh of the asset contains normals and another, using
     * the same material index, does not, they will be brought together, but the
     * first meshes's part of the normal list is zeroed. However, these
     * artifacts are rare.<p>
     * 
     * <b>Note:</b> The <tt>#AI_CONFIG_PP_PTV_NORMALIZE</tt> configuration 
     * property can be set to normalize the scene's spatial dimension to the 
     * -1...1 range.
     */
    PRE_TRANSFORM_VERTICES(0x100),


    /**
     * Limits the number of bones simultaneously affecting a single vertex to a
     * maximum value.<p>
     * 
     * If any vertex is affected by more than the maximum number of bones, the
     * least important vertex weights are removed and the remaining vertex
     * weights are renormalized so that the weights still sum up to 1. The
     * default bone weight limit is 4 (defined as <tt>#AI_LMW_MAX_WEIGHTS</tt>
     * in config.h), but you can use the <tt>#AI_CONFIG_PP_LBW_MAX_WEIGHTS</tt>
     * setting to supply your own limit to the post processing step.<p>
     * 
     * If you intend to perform the skinning in hardware, this post processing
     * step might be of interest to you.
     */
    LIMIT_BONE_WEIGHTS(0x200),


    /**
     * Validates the imported scene data structure. This makes sure that all
     * indices are valid, all animations and bones are linked correctly, all
     * material references are correct .. etc.<p>
     * 
     * It is recommended that you capture Assimp's log output if you use this
     * flag, so you can easily find out what's wrong if a file fails the
     * validation. The validator is quite strict and will find *all*
     * inconsistencies in the data structure... It is recommended that plugin
     * developers use it to debug their loaders. There are two types of
     * validation failures:
     * <ul>
     * <li>Error: There's something wrong with the imported data. Further
     * postprocessing is not possible and the data is not usable at all. The
     * import fails. #Importer::GetErrorString() or #aiGetErrorString() carry
     * the error message around.</li>
     * <li>Warning: There are some minor issues (e.g. 1000000 animation
     * keyframes with the same time), but further postprocessing and use of the
     * data structure is still safe. Warning details are written to the log
     * file, <tt>#AI_SCENE_FLAGS_VALIDATION_WARNING</tt> is set in
     * #aiScene::mFlags</li>
     * </ul>
     * 
     * This post-processing step is not time-consuming. Its use is not
     * compulsory, but recommended.
     */
    VALIDATE_DATA_STRUCTURE(0x400),


    /**
     * Reorders triangles for better vertex cache locality.<p>
     * 
     * The step tries to improve the ACMR (average post-transform vertex cache
     * miss ratio) for all meshes. The implementation runs in O(n) and is
     * roughly based on the 'tipsify' algorithm (see <a href="
     * http://www.cs.princeton.edu/gfx/pubs/Sander_2007_%3ETR/tipsy.pdf">this
     * paper</a>).<p>
     * 
     * If you intend to render huge models in hardware, this step might be of
     * interest to you. The <tt>#AI_CONFIG_PP_ICL_PTCACHE_SIZE</tt>config
     * setting can be used to fine-tune the cache optimization.
     */
    IMPROVE_CACHE_LOCALITY(0x800),


    /**
     * Searches for redundant/unreferenced materials and removes them.<p>
     * 
     * This is especially useful in combination with the
     * #PretransformVertices and #OptimizeMeshes flags. Both
     * join small meshes with equal characteristics, but they can't do their
     * work if two meshes have different materials. Because several material
     * settings are lost during Assimp's import filters, (and because many
     * exporters don't check for redundant materials), huge models often have
     * materials which are are defined several times with exactly the same
     * settings.<p>
     * 
     * Several material settings not contributing to the final appearance of a
     * surface are ignored in all comparisons (e.g. the material name). So, if
     * you're passing additional information through the content pipeline
     * (probably using *magic* material names), don't specify this flag.
     * Alternatively take a look at the <tt>#AI_CONFIG_PP_RRM_EXCLUDE_LIST</tt>
     * setting.
     */
    REMOVE_REDUNDANT_MATERIALS(0x1000),


    /**
     * This step tries to determine which meshes have normal vectors that are
     * facing inwards and inverts them.<p>
     * 
     * The algorithm is simple but effective: the bounding box of all vertices +
     * their normals is compared against the volume of the bounding box of all
     * vertices without their normals. This works well for most objects,
     * problems might occur with planar surfaces. However, the step tries to
     * filter such cases. The step inverts all in-facing normals. Generally it
     * is recommended to enable this step, although the result is not always
     * correct.
     */
    FIX_INFACING_NORMALS(0x2000),


    /**
     * This step splits meshes with more than one primitive type in homogeneous
     * sub-meshes.<p>
     * 
     * The step is executed after the triangulation step. After the step
     * returns, just one bit is set in aiMesh::mPrimitiveTypes. This is
     * especially useful for real-time rendering where point and line primitives
     * are often ignored or rendered separately. You can use the
     * <tt>#AI_CONFIG_PP_SBP_REMOVE</tt> option to specify which primitive types
     * you need. This can be used to easily exclude lines and points, which are
     * rarely used, from the import.
     */
    SORT_BY_PTYPE(0x8000),


    /**
     * This step searches all meshes for degenerate primitives and converts them
     * to proper lines or points.<p>
     * 
     * A face is 'degenerate' if one or more of its points are identical. To
     * have the degenerate stuff not only detected and collapsed but removed,
     * try one of the following procedures: <br>
     * <b>1.</b> (if you support lines and points for rendering but don't want
     * the degenerates)</br>
     * <ul>
     * <li>Specify the #FindDegenerates flag.</li>
     * <li>Set the <tt>AI_CONFIG_PP_FD_REMOVE</tt> option to 1. This will cause
     * the step to remove degenerate triangles from the import as soon as
     * they're detected. They won't pass any further pipeline steps.</li>
     * </ul>
     * <br>
     * <b>2.</b>(if you don't support lines and points at all)</br>
     * <ul>
     *   <li>Specify the #FindDegenerates flag.
     *   <li>Specify the #SortByPType flag. This moves line and point 
     *       primitives to separate meshes.
     *   <li>Set the <tt>AI_CONFIG_PP_SBP_REMOVE</tt> option to
     *       <code>aiPrimitiveType_POINTS | aiPrimitiveType_LINES</code>
     *       to cause SortByPType to reject point and line meshes from the 
     *       scene.
     * </ul>
     * <b>Note:</b> Degenerated polygons are not necessarily evil and that's 
     * why they're not removed by default. There are several file formats 
     * which don't support lines or points, and some exporters bypass the 
     * format specification and write them as degenerate triangles instead.
     */
    FIND_DEGENERATES(0x10000),


    /**
     * This step searches all meshes for invalid data, such as zeroed normal
     * vectors or invalid UV coords and removes/fixes them. This is intended to
     * get rid of some common exporter errors.<p>
     * 
     * This is especially useful for normals. If they are invalid, and the step
     * recognizes this, they will be removed and can later be recomputed, i.e.
     * by the {@link #GEN_SMOOTH_NORMALS} flag.<p>
     * 
     * The step will also remove meshes that are infinitely small and reduce
     * animation tracks consisting of hundreds if redundant keys to a single
     * key. The <tt>AI_CONFIG_PP_FID_ANIM_ACCURACY</tt> config property decides
     * the accuracy of the check for duplicate animation tracks.
     */
    FIND_INVALID_DATA(0x20000),


    /**
     * This step converts non-UV mappings (such as spherical or cylindrical
     * mapping) to proper texture coordinate channels.<p>
     * 
     * Most applications will support UV mapping only, so you will probably want
     * to specify this step in every case. Note that Assimp is not always able
     * to match the original mapping implementation of the 3D app which produced
     * a model perfectly. It's always better to let the modelling app compute
     * the UV channels - 3ds max, Maya, Blender, LightWave, and Modo do this for
     * example.<p>
     * 
     * <b>Note:</b> If this step is not requested, you'll need to process the
     * <tt>MATKEY_MAPPING</tt> material property in order to display all 
     * assets properly.
     */
    GEN_UV_COORDS(0x40000),


    /**
     * This step applies per-texture UV transformations and bakes them into
     * stand-alone vtexture coordinate channels.<p>
     * 
     * UV transformations are specified per-texture - see the
     * <tt>MATKEY_UVTRANSFORM</tt> material key for more information. This
     * step processes all textures with transformed input UV coordinates and
     * generates a new (pre-transformed) UV channel which replaces the old
     * channel. Most applications won't support UV transformations, so you will
     * probably want to specify this step.<p>
     * 
     * <b>Note:</b> UV transformations are usually implemented in real-time 
     * apps by transforming texture coordinates at vertex shader stage with a 
     * 3x3 (homogenous) transformation matrix.
     */
    TRANSFORM_UV_COORDS(0x80000),


    /**
     * This step searches for duplicate meshes and replaces them with references
     * to the first mesh.<p>
     * 
     * This step takes a while, so don't use it if speed is a concern. Its main
     * purpose is to workaround the fact that many export file formats don't
     * support instanced meshes, so exporters need to duplicate meshes. This
     * step removes the duplicates again. Please note that Assimp does not
     * currently support per-node material assignment to meshes, which means
     * that identical meshes with different materials are currently *not*
     * joined, although this is planned for future versions.
     */
    FIND_INSTANCES(0x100000),


    /**
     * A postprocessing step to reduce the number of meshes.<p>
     * 
     * This will, in fact, reduce the number of draw calls.<p>
     * 
     * This is a very effective optimization and is recommended to be used
     * together with #OptimizeGraph, if possible. The flag is fully
     * compatible with both {@link #SPLIT_LARGE_MESHES} and 
     * {@link #SORT_BY_PTYPE}.
     */
    OPTIMIZE_MESHES(0x200000),


    /**
     * A postprocessing step to optimize the scene hierarchy.<p>
     * 
     * Nodes without animations, bones, lights or cameras assigned are collapsed
     * and joined.<p>
     * 
     * Node names can be lost during this step. If you use special 'tag nodes'
     * to pass additional information through your content pipeline, use the
     * <tt>#AI_CONFIG_PP_OG_EXCLUDE_LIST</tt> setting to specify a list of node
     * names you want to be kept. Nodes matching one of the names in this list
     * won't be touched or modified.<p>
     * 
     * Use this flag with caution. Most simple files will be collapsed to a
     * single node, so complex hierarchies are usually completely lost. This is
     * not useful for editor environments, but probably a very effective
     * optimization if you just want to get the model data, convert it to your
     * own format, and render it as fast as possible.<p>
     * 
     * This flag is designed to be used with #OptimizeMeshes for best
     * results.<p>
     * 
     * <b>Note:</b> 'Crappy' scenes with thousands of extremely small meshes 
     * packed in deeply nested nodes exist for almost all file formats.
     * {@link #OPTIMIZE_MESHES} in combination with {@link #OPTIMIZE_GRAPH}
     * usually fixes them all and makes them renderable.
     */
    OPTIMIZE_GRAPH(0x400000),


    /**
     * This step flips all UV coordinates along the y-axis and adjusts material
     * settings and bitangents accordingly.<p>
     * 
     * <b>Output UV coordinate system:</b><br>
     * <code><pre>
     * 0y|0y ---------- 1x|0y 
     *   |                | 
     *   |                |
     *   |                | 
     * 0x|1y ---------- 1x|1y
     * </pre></code>
     * <p>
     *          
     * You'll probably want to consider this flag if you use Direct3D for 
     * rendering. The {@link #MAKE_LEFT_HANDED} flag supersedes this setting 
     * and bundles all conversions typically required for D3D-based 
     * applications.
     */
    FLIP_UVS(0x800000),


    /**
     * This step adjusts the output face winding order to be CW.<p>
     * 
     * The default face winding order is counter clockwise (CCW).
     * 
     * <b>Output face order:</b>
     * 
     * <code><pre>
     *        x2
     * 
     *                      x0 
     *  x1
     * </pre></code>
     */
    FLIP_WINDING_ORDER(0x1000000),


    /**
     * This step splits meshes with many bones into sub-meshes so that each
     * sub-mesh has fewer or as many bones as a given limit.<p>
     */
    SPLIT_BY_BONE_COUNT(0x2000000),


    /**
     * This step removes bones losslessly or according to some threshold.<p>
     * 
     * In some cases (i.e. formats that require it) exporters are forced to
     * assign dummy bone weights to otherwise static meshes assigned to animated
     * meshes. Full, weight-based skinning is expensive while animating nodes is
     * extremely cheap, so this step is offered to clean up the data in that
     * regard.<p>
     * 
     * Use <tt>#AI_CONFIG_PP_DB_THRESHOLD</tt> to control this. Use
     * <tt>#AI_CONFIG_PP_DB_ALL_OR_NONE</tt> if you want bones removed if and
     * only if all bones within the scene qualify for removal.
     */
    DEBONE(0x4000000);

    
    /**
     * Utility method for converting to c/c++ based integer enums from java 
     * enums.<p>
     * 
     * This method is intended to be used from JNI and my change based on
     * implementation needs.
     * 
     * @param set the set to convert
     * @return an integer based enum value (as defined by assimp) 
     */
    static long toRawValue(Set<AiPostProcessSteps> set) {
        long rawValue = 0L;
        
        for (AiPostProcessSteps step : set) {
            rawValue |= step.m_rawValue;
        }
        
        return rawValue;
    }
    
    
    /**
     * Constructor.
     * 
     * @param rawValue maps java enum to c/c++ integer enum values
     */
    private AiPostProcessSteps(long rawValue) {
        m_rawValue = rawValue;
    }

    
    /**
     * The mapped c/c++ integer enum value.
     */
    private final long m_rawValue;
}
