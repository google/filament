#-*- coding: UTF-8 -*-

from ctypes import POINTER, c_void_p, c_int, c_uint, c_char, c_float, Structure, c_char_p, c_double, c_ubyte, c_size_t, c_uint32


class Vector2D(Structure):
    """
    See 'vector2.h' for details.
    """ 


    _fields_ = [
            ("x", c_float),("y", c_float),
        ]

class Matrix3x3(Structure):
    """
    See 'matrix3x3.h' for details.
    """ 


    _fields_ = [
            ("a1", c_float),("a2", c_float),("a3", c_float),
            ("b1", c_float),("b2", c_float),("b3", c_float),
            ("c1", c_float),("c2", c_float),("c3", c_float),
        ]

class Texel(Structure):
    """
    See 'texture.h' for details.
    """ 

    _fields_ = [
            ("b", c_ubyte),("g", c_ubyte),("r", c_ubyte),("a", c_ubyte),
        ]

class Color4D(Structure):
    """
    See 'color4.h' for details.
    """ 


    _fields_ = [
            #  Red, green, blue and alpha color values
            ("r", c_float),("g", c_float),("b", c_float),("a", c_float),
        ]

class Plane(Structure):
    """
    See 'types.h' for details.
    """ 

    _fields_ = [
            #  Plane equation
            ("a", c_float),("b", c_float),("c", c_float),("d", c_float),
        ]

class Color3D(Structure):
    """
    See 'types.h' for details.
    """ 

    _fields_ = [
            #  Red, green and blue color values
            ("r", c_float),("g", c_float),("b", c_float),
        ]

class String(Structure):
    """
    See 'types.h' for details.
    """ 

    MAXLEN = 1024

    _fields_ = [
            # Binary length of the string excluding the terminal 0. This is NOT the
            #  logical length of strings containing UTF-8 multibyte sequences! It's
            #  the number of bytes from the beginning of the string to its end.
            ("length", c_size_t),
            
            # String buffer. Size limit is MAXLEN
            ("data", c_char*MAXLEN),
        ]

class MaterialPropertyString(Structure):
    """
    See 'MaterialSystem.cpp' for details.
    
    The size of length is truncated to 4 bytes on 64-bit platforms when used as a
    material property (see MaterialSystem.cpp aiMaterial::AddProperty() for details).
    """

    MAXLEN = 1024

    _fields_ = [
            # Binary length of the string excluding the terminal 0. This is NOT the
            #  logical length of strings containing UTF-8 multibyte sequences! It's
            #  the number of bytes from the beginning of the string to its end.
            ("length", c_uint32),
            
            # String buffer. Size limit is MAXLEN
            ("data", c_char*MAXLEN),
        ]

class MemoryInfo(Structure):
    """
    See 'types.h' for details.
    """ 

    _fields_ = [
            # Storage allocated for texture data
            ("textures", c_uint),
            
            # Storage allocated for material data
            ("materials", c_uint),
            
            # Storage allocated for mesh data
            ("meshes", c_uint),
            
            # Storage allocated for node data
            ("nodes", c_uint),
            
            # Storage allocated for animation data
            ("animations", c_uint),
            
            # Storage allocated for camera data
            ("cameras", c_uint),
            
            # Storage allocated for light data
            ("lights", c_uint),
            
            # Total storage allocated for the full import.
            ("total", c_uint),
        ]

class Quaternion(Structure):
    """
    See 'quaternion.h' for details.
    """ 


    _fields_ = [
            #  w,x,y,z components of the quaternion
            ("w", c_float),("x", c_float),("y", c_float),("z", c_float),
        ]

class Face(Structure):
    """
    See 'mesh.h' for details.
    """ 

    _fields_ = [
            #  Number of indices defining this face.
            #  The maximum value for this member is
            #AI_MAX_FACE_INDICES.
            ("mNumIndices", c_uint),
            
            #  Pointer to the indices array. Size of the array is given in numIndices.
            ("mIndices", POINTER(c_uint)),
        ]

class VertexWeight(Structure):
    """
    See 'mesh.h' for details.
    """ 

    _fields_ = [
            #  Index of the vertex which is influenced by the bone.
            ("mVertexId", c_uint),
            
            #  The strength of the influence in the range (0...1).
            #  The influence from all bones at one vertex amounts to 1.
            ("mWeight", c_float),
        ]

class Matrix4x4(Structure):
    """
    See 'matrix4x4.h' for details.
    """ 


    _fields_ = [
            ("a1", c_float),("a2", c_float),("a3", c_float),("a4", c_float),
            ("b1", c_float),("b2", c_float),("b3", c_float),("b4", c_float),
            ("c1", c_float),("c2", c_float),("c3", c_float),("c4", c_float),
            ("d1", c_float),("d2", c_float),("d3", c_float),("d4", c_float),
        ]

class Vector3D(Structure):
    """
    See 'vector3.h' for details.
    """ 


    _fields_ = [
            ("x", c_float),("y", c_float),("z", c_float),
        ]

class MeshKey(Structure):
    """
    See 'anim.h' for details.
    """ 

    _fields_ = [
            # The time of this key
            ("mTime", c_double),
            
            # Index into the aiMesh::mAnimMeshes array of the
            #  mesh corresponding to the
            #aiMeshAnim hosting this
            #  key frame. The referenced anim mesh is evaluated
            #  according to the rules defined in the docs for
            #aiAnimMesh.
            ("mValue", c_uint),
        ]

class MetadataEntry(Structure):
    """
    See 'metadata.h' for details
    """
    AI_BOOL       = 0
    AI_INT32      = 1
    AI_UINT64     = 2
    AI_FLOAT      = 3
    AI_DOUBLE     = 4
    AI_AISTRING   = 5
    AI_AIVECTOR3D = 6
    AI_META_MAX   = 7
    _fields_ = [
            # The type field uniquely identifies the underlying type of the data field
            ("mType", c_uint),
            ("mData", c_void_p),
    ]

class Metadata(Structure):
    """
    See 'metadata.h' for details
    """
    _fields_ = [
            # Length of the mKeys and mValues arrays, respectively
            ("mNumProperties", c_uint),

            # Arrays of keys, may not be NULL. Entries in this array may not be NULL
            # as well.
            ("mKeys", POINTER(String)),

            # Arrays of values, may not be NULL. Entries in this array may be NULL
            # if the corresponding property key has no assigned value.
            ("mValues", POINTER(MetadataEntry)),
    ]

class Node(Structure):
    """
    See 'scene.h' for details.
    """ 


Node._fields_ = [
            # The name of the node.
            # The name might be empty (length of zero) but all nodes which
            # need to be accessed afterwards by bones or anims are usually named.
            # Multiple nodes may have the same name, but nodes which are accessed
            # by bones (see
            #aiBone and
            #aiMesh::mBones) *must* be unique.
            # Cameras and lights are assigned to a specific node name - if there
            # are multiple nodes with this name, they're assigned to each of them.
            # <br>
            # There are no limitations regarding the characters contained in
            # this text. You should be able to handle stuff like whitespace, tabs,
            # linefeeds, quotation marks, ampersands, ... .
            ("mName", String),
            
            # The transformation relative to the node's parent.
            ("mTransformation", Matrix4x4),
            
            # Parent node. NULL if this node is the root node.
            ("mParent", POINTER(Node)),
            
            # The number of child nodes of this node.
            ("mNumChildren", c_uint),
            
            # The child nodes of this node. NULL if mNumChildren is 0.
            ("mChildren", POINTER(POINTER(Node))),
            
            # The number of meshes of this node.
            ("mNumMeshes", c_uint),
            
            # The meshes of this node. Each entry is an index into the mesh
            ("mMeshes", POINTER(c_uint)),

            # Metadata associated with this node or NULL if there is no metadata.
            # Whether any metadata is generated depends on the source file format.
            ("mMetadata", POINTER(Metadata)),
        ]

class Light(Structure):
    """
    See 'light.h' for details.
    """ 


    _fields_ = [
            # The name of the light source.
            #  There must be a node in the scenegraph with the same name.
            #  This node specifies the position of the light in the scene
            #  hierarchy and can be animated.
            ("mName", String),
            
            # The type of the light source.
            # aiLightSource_UNDEFINED is not a valid value for this member.
            ("mType", c_uint),
            
            # Position of the light source in space. Relative to the
            #  transformation of the node corresponding to the light.
            #  The position is undefined for directional lights.
            ("mPosition", Vector3D),
            
            # Direction of the light source in space. Relative to the
            #  transformation of the node corresponding to the light.
            #  The direction is undefined for point lights. The vector
            #  may be normalized, but it needn't.
            ("mDirection", Vector3D),
            
            # Up direction of the light source in space. Relative to the
            #  transformation of the node corresponding to the light.
            #
            # The direction is undefined for point lights. The vector
            #  may be normalized, but it needn't.
            ("mUp", Vector3D),

            # Constant light attenuation factor.
            #  The intensity of the light source at a given distance 'd' from
            #  the light's position is
            #  @code
            #  Atten = 1/( att0 + att1
            # d + att2
            # d*d)
            #  @endcode
            #  This member corresponds to the att0 variable in the equation.
            #  Naturally undefined for directional lights.
            ("mAttenuationConstant", c_float),
            
            # Linear light attenuation factor.
            #  The intensity of the light source at a given distance 'd' from
            #  the light's position is
            #  @code
            #  Atten = 1/( att0 + att1
            # d + att2
            # d*d)
            #  @endcode
            #  This member corresponds to the att1 variable in the equation.
            #  Naturally undefined for directional lights.
            ("mAttenuationLinear", c_float),
            
            # Quadratic light attenuation factor.
            #  The intensity of the light source at a given distance 'd' from
            #  the light's position is
            #  @code
            #  Atten = 1/( att0 + att1
            # d + att2
            # d*d)
            #  @endcode
            #  This member corresponds to the att2 variable in the equation.
            #  Naturally undefined for directional lights.
            ("mAttenuationQuadratic", c_float),
            
            # Diffuse color of the light source
            #  The diffuse light color is multiplied with the diffuse
            #  material color to obtain the final color that contributes
            #  to the diffuse shading term.
            ("mColorDiffuse", Color3D),
            
            # Specular color of the light source
            #  The specular light color is multiplied with the specular
            #  material color to obtain the final color that contributes
            #  to the specular shading term.
            ("mColorSpecular", Color3D),
            
            # Ambient color of the light source
            #  The ambient light color is multiplied with the ambient
            #  material color to obtain the final color that contributes
            #  to the ambient shading term. Most renderers will ignore
            #  this value it, is just a remaining of the fixed-function pipeline
            #  that is still supported by quite many file formats.
            ("mColorAmbient", Color3D),
            
            # Inner angle of a spot light's light cone.
            #  The spot light has maximum influence on objects inside this
            #  angle. The angle is given in radians. It is 2PI for point
            #  lights and undefined for directional lights.
            ("mAngleInnerCone", c_float),
            
            # Outer angle of a spot light's light cone.
            #  The spot light does not affect objects outside this angle.
            #  The angle is given in radians. It is 2PI for point lights and
            #  undefined for directional lights. The outer angle must be
            #  greater than or equal to the inner angle.
            #  It is assumed that the application uses a smooth
            #  interpolation between the inner and the outer cone of the
            #  spot light.
            ("mAngleOuterCone", c_float),

            # Size of area light source.
            ("mSize", Vector2D),
        ]

class Texture(Structure):
    """
    See 'texture.h' for details.
    """ 


    _fields_ = [
            # Width of the texture, in pixels
            # If mHeight is zero the texture is compressed in a format
            # like JPEG. In this case mWidth specifies the size of the
            # memory area pcData is pointing to, in bytes.
            ("mWidth", c_uint),
            
            # Height of the texture, in pixels
            # If this value is zero, pcData points to an compressed texture
            # in any format (e.g. JPEG).
            ("mHeight", c_uint),
            
            # A hint from the loader to make it easier for applications
            # to determine the type of embedded textures.
            # 
            # If mHeight != 0 this member is show how data is packed. Hint will consist of
            # two parts: channel order and channel bitness (count of the bits for every
            # color channel). For simple parsing by the viewer it's better to not omit
            # absent color channel and just use 0 for bitness. For example:
            # 1. Image contain RGBA and 8 bit per channel, achFormatHint == "rgba8888";
            # 2. Image contain ARGB and 8 bit per channel, achFormatHint == "argb8888";
            # 3. Image contain RGB and 5 bit for R and B channels and 6 bit for G channel,
            #    achFormatHint == "rgba5650";
            # 4. One color image with B channel and 1 bit for it, achFormatHint == "rgba0010";
            # If mHeight == 0 then achFormatHint is set set to '\\0\\0\\0\\0' if the loader has no additional
            # information about the texture file format used OR the
            # file extension of the format without a trailing dot. If there
            # are multiple file extensions for a format, the shortest
            # extension is chosen (JPEG maps to 'jpg', not to 'jpeg').
            # E.g. 'dds\\0', 'pcx\\0', 'jpg\\0'.  All characters are lower-case.
            # The fourth character will always be '\\0'.
            ("achFormatHint", c_char*9),
            
            # Data of the texture.
            # Points to an array of mWidth
            # mHeight aiTexel's.
            # The format of the texture data is always ARGB8888 to
            # make the implementation for user of the library as easy
            # as possible. If mHeight = 0 this is a pointer to a memory
            # buffer of size mWidth containing the compressed texture
            # data. Good luck, have fun!
            ("pcData", POINTER(Texel)),

            # Texture original filename
            # Used to get the texture reference
            ("mFilename", String),
        ]

class Ray(Structure):
    """
    See 'types.h' for details.
    """ 

    _fields_ = [
            #  Position and direction of the ray
            ("pos", Vector3D),("dir", Vector3D),
        ]

class UVTransform(Structure):
    """
    See 'material.h' for details.
    """ 

    _fields_ = [
            # Translation on the u and v axes.
            #  The default value is (0|0).
            ("mTranslation", Vector2D),
            
            # Scaling on the u and v axes.
            #  The default value is (1|1).
            ("mScaling", Vector2D),
            
            # Rotation - in counter-clockwise direction.
            #  The rotation angle is specified in radians. The
            #  rotation center is 0.5f|0.5f. The default value
            #  0.f.
            ("mRotation", c_float),
        ]

class MaterialProperty(Structure):
    """
    See 'material.h' for details.
    """ 

    _fields_ = [
            # Specifies the name of the property (key)
            #  Keys are generally case insensitive.
            ("mKey", String),
            
            # Textures: Specifies their exact usage semantic.
            # For non-texture properties, this member is always 0
            # (or, better-said,
            #aiTextureType_NONE).
            ("mSemantic", c_uint),
            
            # Textures: Specifies the index of the texture.
            #  For non-texture properties, this member is always 0.
            ("mIndex", c_uint),
            
            # Size of the buffer mData is pointing to, in bytes.
            #  This value may not be 0.
            ("mDataLength", c_uint),
            
            # Type information for the property.
            # Defines the data layout inside the data buffer. This is used
            # by the library internally to perform debug checks and to
            # utilize proper type conversions.
            # (It's probably a hacky solution, but it works.)
            ("mType", c_uint),
            
            # Binary buffer to hold the property's value.
            # The size of the buffer is always mDataLength.
            ("mData", POINTER(c_char)),
        ]

class Material(Structure):
    """
    See 'material.h' for details.
    """ 

    _fields_ = [
            # List of all material properties loaded.
            ("mProperties", POINTER(POINTER(MaterialProperty))),
            
            # Number of properties in the data base
            ("mNumProperties", c_uint),
            
            # Storage allocated
            ("mNumAllocated", c_uint),
        ]

class Bone(Structure):
    """
    See 'mesh.h' for details.
    """ 

    _fields_ = [
            #  The name of the bone.
            ("mName", String),
            
            #  The number of vertices affected by this bone
            #  The maximum value for this member is
            #AI_MAX_BONE_WEIGHTS.
            ("mNumWeights", c_uint),
            
            #  The vertices affected by this bone
            ("mWeights", POINTER(VertexWeight)),
            
            #  Matrix that transforms from mesh space to bone space in bind pose
            ("mOffsetMatrix", Matrix4x4),
        ]


class AnimMesh(Structure):
    """
    See 'mesh.h' for details.
    """ 

    AI_MAX_NUMBER_OF_TEXTURECOORDS = 0x8
    AI_MAX_NUMBER_OF_COLOR_SETS = 0x8

    _fields_ = [
            # Replacement for aiMesh::mVertices. If this array is non-NULL,
            # it *must* contain mNumVertices entries. The corresponding
            # array in the host mesh must be non-NULL as well - animation
            # meshes may neither add or nor remove vertex components (if
            # a replacement array is NULL and the corresponding source
            # array is not, the source data is taken instead)
            ("mVertices", POINTER(Vector3D)),

            # Replacement for aiMesh::mNormals.
            ("mNormals", POINTER(Vector3D)),

            # Replacement for aiMesh::mTangents.
            ("mTangents", POINTER(Vector3D)),

            # Replacement for aiMesh::mBitangents.
            ("mBitangents", POINTER(Vector3D)),

            # Replacement for aiMesh::mColors
            ("mColors", POINTER(Color4D) * AI_MAX_NUMBER_OF_COLOR_SETS),

            # Replacement for aiMesh::mTextureCoords
            ("mTextureCoords", POINTER(Vector3D) * AI_MAX_NUMBER_OF_TEXTURECOORDS),

            # The number of vertices in the aiAnimMesh, and thus the length of all
            # the member arrays.
            #
            # This has always the same value as the mNumVertices property in the
            # corresponding aiMesh. It is duplicated here merely to make the length
            # of the member arrays accessible even if the aiMesh is not known, e.g.
            # from language bindings.
            ("mNumVertices", c_uint),

            # Weight of the AnimMesh.
            ("mWeight", c_float),
        ]


class Mesh(Structure):
    """
    See 'mesh.h' for details.
    """ 

    AI_MAX_FACE_INDICES = 0x7fff
    AI_MAX_BONE_WEIGHTS = 0x7fffffff
    AI_MAX_VERTICES = 0x7fffffff
    AI_MAX_FACES = 0x7fffffff
    AI_MAX_NUMBER_OF_COLOR_SETS = 0x8
    AI_MAX_NUMBER_OF_TEXTURECOORDS = 0x8

    _fields_ = [ # Bitwise combination of the members of the
            #aiPrimitiveType enum.
            # This specifies which types of primitives are present in the mesh.
            # The "SortByPrimitiveType"-Step can be used to make sure the
            # output meshes consist of one primitive type each.
            ("mPrimitiveTypes", c_uint),
            
            # The number of vertices in this mesh.
            # This is also the size of all of the per-vertex data arrays.
            # The maximum value for this member is
            #AI_MAX_VERTICES.
            ("mNumVertices", c_uint),
            
            # The number of primitives (triangles, polygons, lines) in this  mesh.
            # This is also the size of the mFaces array.
            # The maximum value for this member is
            #AI_MAX_FACES.
            ("mNumFaces", c_uint),
            
            # Vertex positions.
            # This array is always present in a mesh. The array is
            # mNumVertices in size.
            ("mVertices", POINTER(Vector3D)),
            
            # Vertex normals.
            # The array contains normalized vectors, NULL if not present.
            # The array is mNumVertices in size. Normals are undefined for
            # point and line primitives. A mesh consisting of points and
            # lines only may not have normal vectors. Meshes with mixed
            # primitive types (i.e. lines and triangles) may have normals,
            # but the normals for vertices that are only referenced by
            # point or line primitives are undefined and set to QNaN (WARN:
            # qNaN compares to inequal to *everything*, even to qNaN itself.
            # Using code like this to check whether a field is qnan is:
            # @code
            #define IS_QNAN(f) (f != f)
            # @endcode
            # still dangerous because even 1.f == 1.f could evaluate to false! (
            # remember the subtleties of IEEE754 artithmetics). Use stuff like
            # @c fpclassify instead.
            # @note Normal vectors computed by Assimp are always unit-length.
            # However, this needn't apply for normals that have been taken
            #   directly from the model file.
            ("mNormals", POINTER(Vector3D)),
            
            # Vertex tangents.
            # The tangent of a vertex points in the direction of the positive
            # X texture axis. The array contains normalized vectors, NULL if
            # not present. The array is mNumVertices in size. A mesh consisting
            # of points and lines only may not have normal vectors. Meshes with
            # mixed primitive types (i.e. lines and triangles) may have
            # normals, but the normals for vertices that are only referenced by
            # point or line primitives are undefined and set to qNaN.  See
            # the
            #mNormals member for a detailed discussion of qNaNs.
            # @note If the mesh contains tangents, it automatically also
            # contains bitangents (the bitangent is just the cross product of
            # tangent and normal vectors).
            ("mTangents", POINTER(Vector3D)),
            
            # Vertex bitangents.
            # The bitangent of a vertex points in the direction of the positive
            # Y texture axis. The array contains normalized vectors, NULL if not
            # present. The array is mNumVertices in size.
            # @note If the mesh contains tangents, it automatically also contains
            # bitangents.
            ("mBitangents", POINTER(Vector3D)),
            
            # Vertex color sets.
            # A mesh may contain 0 to
            #AI_MAX_NUMBER_OF_COLOR_SETS vertex
            # colors per vertex. NULL if not present. Each array is
            # mNumVertices in size if present.
            ("mColors", POINTER(Color4D)*AI_MAX_NUMBER_OF_COLOR_SETS),
            
            # Vertex texture coords, also known as UV channels.
            # A mesh may contain 0 to AI_MAX_NUMBER_OF_TEXTURECOORDS per
            # vertex. NULL if not present. The array is mNumVertices in size.
            ("mTextureCoords", POINTER(Vector3D)*AI_MAX_NUMBER_OF_TEXTURECOORDS),
            
            # Specifies the number of components for a given UV channel.
            # Up to three channels are supported (UVW, for accessing volume
            # or cube maps). If the value is 2 for a given channel n, the
            # component p.z of mTextureCoords[n][p] is set to 0.0f.
            # If the value is 1 for a given channel, p.y is set to 0.0f, too.
            # @note 4D coords are not supported
            ("mNumUVComponents", c_uint*AI_MAX_NUMBER_OF_TEXTURECOORDS),
            
            # The faces the mesh is constructed from.
            # Each face refers to a number of vertices by their indices.
            # This array is always present in a mesh, its size is given
            # in mNumFaces. If the
            #AI_SCENE_FLAGS_NON_VERBOSE_FORMAT
            # is NOT set each face references an unique set of vertices.
            ("mFaces", POINTER(Face)),
            
            # The number of bones this mesh contains.
            # Can be 0, in which case the mBones array is NULL.
            ("mNumBones", c_uint),
            
            # The bones of this mesh.
            # A bone consists of a name by which it can be found in the
            # frame hierarchy and a set of vertex weights.
            ("mBones", POINTER(POINTER(Bone))),
            
            # The material used by this mesh.
            # A mesh does use only a single material. If an imported model uses
            # multiple materials, the import splits up the mesh. Use this value
            # as index into the scene's material list.
            ("mMaterialIndex", c_uint),
            
            # Name of the mesh. Meshes can be named, but this is not a
            #  requirement and leaving this field empty is totally fine.
            #  There are mainly three uses for mesh names:
            #   - some formats name nodes and meshes independently.
            #   - importers tend to split meshes up to meet the
            #      one-material-per-mesh requirement. Assigning
            #      the same (dummy) name to each of the result meshes
            #      aids the caller at recovering the original mesh
            #      partitioning.
            #   - Vertex animations refer to meshes by their names.
            ("mName", String),
            
            # The number of attachment meshes. Note! Currently only works with Collada loader.
            ("mNumAnimMeshes", c_uint),
            
            # Attachment meshes for this mesh, for vertex-based animation.
            # Attachment meshes carry replacement data for some of the
            # mesh'es vertex components (usually positions, normals).
            # Note! Currently only works with Collada loader.
            ("mAnimMesh", POINTER(POINTER(AnimMesh))),

            # Method of morphing when animeshes are specified.
            ("mMethod", c_uint),

        ]

class Camera(Structure):
    """
    See 'camera.h' for details.
    """ 


    _fields_ = [
            # The name of the camera.
            #  There must be a node in the scenegraph with the same name.
            #  This node specifies the position of the camera in the scene
            #  hierarchy and can be animated.
            ("mName", String),
            
            # Position of the camera relative to the coordinate space
            #  defined by the corresponding node.
            #  The default value is 0|0|0.
            ("mPosition", Vector3D),
            
            # 'Up' - vector of the camera coordinate system relative to
            #  the coordinate space defined by the corresponding node.
            #  The 'right' vector of the camera coordinate system is
            #  the cross product of  the up and lookAt vectors.
            #  The default value is 0|1|0. The vector
            #  may be normalized, but it needn't.
            ("mUp", Vector3D),
            
            # 'LookAt' - vector of the camera coordinate system relative to
            #  the coordinate space defined by the corresponding node.
            #  This is the viewing direction of the user.
            #  The default value is 0|0|1. The vector
            #  may be normalized, but it needn't.
            ("mLookAt", Vector3D),
            
            # Half horizontal field of view angle, in radians.
            #  The field of view angle is the angle between the center
            #  line of the screen and the left or right border.
            #  The default value is 1/4PI.
            ("mHorizontalFOV", c_float),
            
            # Distance of the near clipping plane from the camera.
            # The value may not be 0.f (for arithmetic reasons to prevent
            # a division through zero). The default value is 0.1f.
            ("mClipPlaneNear", c_float),
            
            # Distance of the far clipping plane from the camera.
            # The far clipping plane must, of course, be further away than the
            # near clipping plane. The default value is 1000.f. The ratio
            # between the near and the far plane should not be too
            # large (between 1000-10000 should be ok) to avoid floating-point
            # inaccuracies which could lead to z-fighting.
            ("mClipPlaneFar", c_float),
            
            # Screen aspect ratio.
            # This is the ration between the width and the height of the
            # screen. Typical values are 4/3, 1/2 or 1/1. This value is
            # 0 if the aspect ratio is not defined in the source file.
            # 0 is also the default value.
            ("mAspect", c_float),
        ]

class VectorKey(Structure):
    """
    See 'anim.h' for details.
    """ 

    _fields_ = [
            # The time of this key
            ("mTime", c_double),
            
            # The value of this key
            ("mValue", Vector3D),
        ]

class QuatKey(Structure):
    """
    See 'anim.h' for details.
    """ 

    _fields_ = [
            # The time of this key
            ("mTime", c_double),
            
            # The value of this key
            ("mValue", Quaternion),
        ]

class MeshMorphKey(Structure):
    """
    See 'anim.h' for details.
    """ 

    _fields_ = [
            # The time of this key
            ("mTime", c_double),

            # The values and weights at the time of this key
            ("mValues", POINTER(c_uint)),
            ("mWeights", POINTER(c_double)),

            # The number of values and weights
            ("mNumValuesAndWeights", c_uint),

        ]

class NodeAnim(Structure):
    """
    See 'anim.h' for details.
    """ 

    _fields_ = [
            # The name of the node affected by this animation. The node
            #  must exist and it must be unique.
            ("mNodeName", String),
            
            # The number of position keys
            ("mNumPositionKeys", c_uint),
            
            # The position keys of this animation channel. Positions are
            # specified as 3D vector. The array is mNumPositionKeys in size.
            # If there are position keys, there will also be at least one
            # scaling and one rotation key.
            ("mPositionKeys", POINTER(VectorKey)),
            
            # The number of rotation keys
            ("mNumRotationKeys", c_uint),
            
            # The rotation keys of this animation channel. Rotations are
            #  given as quaternions,  which are 4D vectors. The array is
            #  mNumRotationKeys in size.
            # If there are rotation keys, there will also be at least one
            # scaling and one position key.
            ("mRotationKeys", POINTER(QuatKey)),
            
            # The number of scaling keys
            ("mNumScalingKeys", c_uint),
            
            # The scaling keys of this animation channel. Scalings are
            #  specified as 3D vector. The array is mNumScalingKeys in size.
            # If there are scaling keys, there will also be at least one
            # position and one rotation key.
            ("mScalingKeys", POINTER(VectorKey)),
            
            # Defines how the animation behaves before the first
            #  key is encountered.
            #  The default value is aiAnimBehaviour_DEFAULT (the original
            #  transformation matrix of the affected node is used).
            ("mPreState", c_uint),
            
            # Defines how the animation behaves after the last
            #  key was processed.
            #  The default value is aiAnimBehaviour_DEFAULT (the original
            #  transformation matrix of the affected node is taken).
            ("mPostState", c_uint),
        ]

class MeshAnim(Structure):
    """
    See 'anim.h' for details.
    """ 

    _fields_ = [
            # Name of the mesh to be animated. An empty string is not allowed,
            #  animated meshes need to be named (not necessarily uniquely,
            #  the name can basically serve as wild-card to select a group
            #  of meshes with similar animation setup)
            ("mName", String),

            # Size of the #mKeys array. Must be 1, at least.
            ("mNumKeys", c_uint),

            # Key frames of the animation. May not be NULL.
            ("mKeys", POINTER(MeshKey)),
        ]

class MeshMorphAnim(Structure):
    """
    See 'anim.h' for details.
    """     

    _fields_ = [
            # Name of the mesh to be animated. An empty string is not allowed,
            # animated meshes need to be named (not necessarily uniquely,
            # the name can basically serve as wildcard to select a group
            # of meshes with similar animation setup)
            ("mName", String),

            # Size of the #mKeys array. Must be 1, at least.
            ("mNumKeys", c_uint),

            # Key frames of the animation. May not be NULL.
            ("mKeys", POINTER(MeshMorphKey)),
        ]


class Animation(Structure):
    """
    See 'anim.h' for details.
    """ 

    _fields_ = [
            # The name of the animation. If the modeling package this data was
            #  exported from does support only a single animation channel, this
            #  name is usually empty (length is zero).
            ("mName", String),
            
            # Duration of the animation in ticks.
            ("mDuration", c_double),
            
            # Ticks per second. 0 if not specified in the imported file
            ("mTicksPerSecond", c_double),
            
            # The number of bone animation channels. Each channel affects
            #  a single node.
            ("mNumChannels", c_uint),
            
            # The node animation channels. Each channel affects a single node.
            #  The array is mNumChannels in size.
            ("mChannels", POINTER(POINTER(NodeAnim))),
            
            # The number of mesh animation channels. Each channel affects
            #  a single mesh and defines vertex-based animation.
            ("mNumMeshChannels", c_uint),
            
            # The mesh animation channels. Each channel affects a single mesh.
            #  The array is mNumMeshChannels in size.
            ("mMeshChannels", POINTER(POINTER(MeshAnim))),

            # The number of mesh animation channels. Each channel affects
            # a single mesh and defines morphing animation.
            ("mNumMorphMeshChannels", c_uint),

            # The morph mesh animation channels. Each channel affects a single mesh.
            # The array is mNumMorphMeshChannels in size. 
            ("mMorphMeshChannels", POINTER(POINTER(MeshMorphAnim))),

        ]

class ExportDataBlob(Structure):
    """
    See 'cexport.h' for details.

    Note that the '_fields_' definition is outside the class to allow the 'next' field to be recursive
    """
    pass

ExportDataBlob._fields_ = [
            # Size of the data in bytes
            ("size", c_size_t),

            # The data.
            ("data", c_void_p),

            # Name of the blob. An empty string always
            # indicates the first (and primary) blob,
            # which contains the actual file data.
            # Any other blobs are auxiliary files produced
            # by exporters (i.e. material files). Existence
            # of such files depends on the file format. Most
            # formats don't split assets across multiple files.
            #
            # If used, blob names usually contain the file
            # extension that should be used when writing
            # the data to disc.
            ("name", String),

            # Pointer to the next blob in the chain or NULL if there is none.
            ("next", POINTER(ExportDataBlob)),
        ]


class Scene(Structure):
    """
    See 'aiScene.h' for details.
    """ 

    AI_SCENE_FLAGS_INCOMPLETE = 0x1
    AI_SCENE_FLAGS_VALIDATED = 0x2
    AI_SCENE_FLAGS_VALIDATION_WARNING =  	0x4
    AI_SCENE_FLAGS_NON_VERBOSE_FORMAT =  	0x8
    AI_SCENE_FLAGS_TERRAIN = 0x10
    AI_SCENE_FLAGS_ALLOW_SHARED = 0x20

    _fields_ = [
            # Any combination of the AI_SCENE_FLAGS_XXX flags. By default
            # this value is 0, no flags are set. Most applications will
            # want to reject all scenes with the AI_SCENE_FLAGS_INCOMPLETE
            # bit set.
            ("mFlags", c_uint),
            
            # The root node of the hierarchy.
            # There will always be at least the root node if the import
            # was successful (and no special flags have been set).
            # Presence of further nodes depends on the format and content
            # of the imported file.
            ("mRootNode", POINTER(Node)),
            
            # The number of meshes in the scene.
            ("mNumMeshes", c_uint),
            
            # The array of meshes.
            # Use the indices given in the aiNode structure to access
            # this array. The array is mNumMeshes in size. If the
            # AI_SCENE_FLAGS_INCOMPLETE flag is not set there will always
            # be at least ONE material.
            ("mMeshes", POINTER(POINTER(Mesh))),
            
            # The number of materials in the scene.
            ("mNumMaterials", c_uint),
            
            # The array of materials.
            # Use the index given in each aiMesh structure to access this
            # array. The array is mNumMaterials in size. If the
            # AI_SCENE_FLAGS_INCOMPLETE flag is not set there will always
            # be at least ONE material.
            ("mMaterials", POINTER(POINTER(Material))),
            
            # The number of animations in the scene.
            ("mNumAnimations", c_uint),
            
            # The array of animations.
            # All animations imported from the given file are listed here.
            # The array is mNumAnimations in size.
            ("mAnimations", POINTER(POINTER(Animation))),
            
            # The number of textures embedded into the file
            ("mNumTextures", c_uint),
            
            # The array of embedded textures.
            # Not many file formats embed their textures into the file.
            # An example is Quake's MDL format (which is also used by
            # some GameStudio versions)
            ("mTextures", POINTER(POINTER(Texture))),
            
            # The number of light sources in the scene. Light sources
            # are fully optional, in most cases this attribute will be 0
            ("mNumLights", c_uint),
            
            # The array of light sources.
            # All light sources imported from the given file are
            # listed here. The array is mNumLights in size.
            ("mLights", POINTER(POINTER(Light))),
            
            # The number of cameras in the scene. Cameras
            # are fully optional, in most cases this attribute will be 0
            ("mNumCameras", c_uint),
            
            # The array of cameras.
            # All cameras imported from the given file are listed here.
            # The array is mNumCameras in size. The first camera in the
            # array (if existing) is the default camera view into
            # the scene.
            ("mCameras", POINTER(POINTER(Camera))),

            # This data contains global metadata which belongs to the scene like
            # unit-conversions, versions, vendors or other model-specific data. This
            # can be used to store format-specific metadata as well.
            ("mMetadata", POINTER(Metadata)),

            # Internal data, do not touch
            ("mPrivate", c_char_p),
        ]

assimp_structs_as_tuple = (Matrix4x4,
                           Matrix3x3,
                           Vector2D,
                           Vector3D,
                           Color3D,
                           Color4D,
                           Quaternion,
                           Plane,
                           Texel)
