"""
PyAssimp

This is the main-module of PyAssimp.
"""

import sys
if sys.version_info < (2,6):
    raise RuntimeError('pyassimp: need python 2.6 or newer')

# xrange was renamed range in Python 3 and the original range from Python 2 was removed.
# To keep compatibility with both Python 2 and 3, xrange is set to range for version 3.0 and up.
if sys.version_info >= (3,0):
    xrange = range


try: import numpy
except ImportError: numpy = None
import logging
import ctypes
logger = logging.getLogger("pyassimp")
# attach default null handler to logger so it doesn't complain
# even if you don't attach another handler to logger
logger.addHandler(logging.NullHandler())

from . import structs
from . import helper
from . import postprocess
from .errors import AssimpError

class AssimpLib(object):
    """
    Assimp-Singleton
    """
    load, load_mem, export, export_blob, release, dll = helper.search_library()
_assimp_lib = AssimpLib()

def make_tuple(ai_obj, type = None):
    res = None

    #notes:
    # ai_obj._fields_ = [ ("attr", c_type), ... ]
    # getattr(ai_obj, e[0]).__class__ == float

    if isinstance(ai_obj, structs.Matrix4x4):
        if numpy:
            res = numpy.array([getattr(ai_obj, e[0]) for e in ai_obj._fields_]).reshape((4,4))
            #import pdb;pdb.set_trace()
        else:
            res = [getattr(ai_obj, e[0]) for e in ai_obj._fields_]
            res = [res[i:i+4] for i in xrange(0,16,4)]
    elif isinstance(ai_obj, structs.Matrix3x3):
        if numpy:
            res = numpy.array([getattr(ai_obj, e[0]) for e in ai_obj._fields_]).reshape((3,3))
        else:
            res = [getattr(ai_obj, e[0]) for e in ai_obj._fields_]
            res = [res[i:i+3] for i in xrange(0,9,3)]
    else:
        if numpy:
            res = numpy.array([getattr(ai_obj, e[0]) for e in ai_obj._fields_])
        else:
            res = [getattr(ai_obj, e[0]) for e in ai_obj._fields_]

    return res

# Returns unicode object for Python 2, and str object for Python 3.
def _convert_assimp_string(assimp_string):
    if sys.version_info >= (3, 0):
        return str(assimp_string.data, errors='ignore')
    else:
        return unicode(assimp_string.data, errors='ignore')

# It is faster and more correct to have an init function for each assimp class
def _init_face(aiFace):
    aiFace.indices = [aiFace.mIndices[i] for i in range(aiFace.mNumIndices)]
assimp_struct_inits =  { structs.Face : _init_face }

def call_init(obj, caller = None):
    if helper.hasattr_silent(obj,'contents'): #pointer
        _init(obj.contents, obj, caller)
    else:
        _init(obj,parent=caller)

def _is_init_type(obj):

    if obj and helper.hasattr_silent(obj,'contents'): #pointer
        return _is_init_type(obj[0])
    # null-pointer case that arises when we reach a mesh attribute
    # like mBitangents which use mNumVertices rather than mNumBitangents
    # so it breaks the 'is iterable' check.
    # Basically:
    # FIXME!
    elif not bool(obj):
        return False
    tname = obj.__class__.__name__
    return not (tname[:2] == 'c_' or tname == 'Structure' \
            or tname == 'POINTER') and not isinstance(obj, (int, str, bytes))

def _init(self, target = None, parent = None):
    """
    Custom initialize() for C structs, adds safely accessible member functionality.

    :param target: set the object which receive the added methods. Useful when manipulating
    pointers, to skip the intermediate 'contents' deferencing.
    """
    if not target:
        target = self

    dirself = dir(self)
    for m in dirself:

        if m.startswith("_"):
            continue

        if m.startswith('mNum'):
            if 'm' + m[4:] in dirself:
                continue # will be processed later on
            else:
                name = m[1:].lower()

                obj = getattr(self, m)
                setattr(target, name, obj)
                continue

        if m == 'mName':
            target.name = str(_convert_assimp_string(self.mName))
            target.__class__.__repr__ = lambda x: str(x.__class__) + "(" + getattr(x, 'name','') + ")"
            target.__class__.__str__ = lambda x: getattr(x, 'name', '')
            continue

        name = m[1:].lower()

        obj = getattr(self, m)

        # Create tuples
        if isinstance(obj, structs.assimp_structs_as_tuple):
            setattr(target, name, make_tuple(obj))
            logger.debug(str(self) + ": Added array " + str(getattr(target, name)) +  " as self." + name.lower())
            continue

        if m.startswith('m'):

            if name == "parent":
                setattr(target, name, parent)
                logger.debug("Added a parent as self." + name)
                continue

            if helper.hasattr_silent(self, 'mNum' + m[1:]):

                length =  getattr(self, 'mNum' + m[1:])

                # -> special case: properties are
                # stored as a dict.
                if m == 'mProperties':
                    setattr(target, name, _get_properties(obj, length))
                    continue


                if not length: # empty!
                    setattr(target, name, [])
                    logger.debug(str(self) + ": " + name + " is an empty list.")
                    continue


                try:
                    if obj._type_ in structs.assimp_structs_as_tuple:
                        if numpy:
                            setattr(target, name, numpy.array([make_tuple(obj[i]) for i in range(length)], dtype=numpy.float32))

                            logger.debug(str(self) + ": Added an array of numpy arrays (type "+ str(type(obj)) + ") as self." + name)
                        else:
                            setattr(target, name, [make_tuple(obj[i]) for i in range(length)])

                            logger.debug(str(self) + ": Added a list of lists (type "+ str(type(obj)) + ") as self." + name)

                    else:
                        setattr(target, name, [obj[i] for i in range(length)]) #TODO: maybe not necessary to recreate an array?

                        logger.debug(str(self) + ": Added list of " + str(obj) + " " + name + " as self." + name + " (type: " + str(type(obj)) + ")")

                        # initialize array elements
                        try:
                            init = assimp_struct_inits[type(obj[0])]
                        except KeyError:
                            if _is_init_type(obj[0]):
                                for e in getattr(target, name):
                                    call_init(e, target)
                        else:
                            for e in getattr(target, name):
                                init(e)


                except IndexError:
                    logger.error("in " + str(self) +" : mismatch between mNum" + name + " and the actual amount of data in m" + name + ". This may be due to version mismatch between libassimp and pyassimp. Quitting now.")
                    sys.exit(1)

                except ValueError as e:

                    logger.error("In " + str(self) +  "->" + name + ": " + str(e) + ". Quitting now.")
                    if "setting an array element with a sequence" in str(e):
                        logger.error("Note that pyassimp does not currently "
                                     "support meshes with mixed triangles "
                                     "and quads. Try to load your mesh with"
                                     " a post-processing to triangulate your"
                                     " faces.")
                    raise e



            else: # starts with 'm' but not iterable
                setattr(target, name, obj)
                logger.debug("Added " + name + " as self." + name + " (type: " + str(type(obj)) + ")")

                if _is_init_type(obj):
                    call_init(obj, target)

    if isinstance(self, structs.Mesh):
        _finalize_mesh(self, target)

    if isinstance(self, structs.Texture):
        _finalize_texture(self, target)

    if isinstance(self, structs.Metadata):
        _finalize_metadata(self, target)


    return self


def pythonize_assimp(type, obj, scene):
    """ This method modify the Assimp data structures
    to make them easier to work with in Python.

    Supported operations:
     - MESH: replace a list of mesh IDs by reference to these meshes
     - ADDTRANSFORMATION: add a reference to an object's transformation taken from their associated node.

    :param type: the type of modification to operate (cf above)
    :param obj: the input object to modify
    :param scene: a reference to the whole scene
    """

    if type == "MESH":
        meshes = []
        for i in obj:
            meshes.append(scene.meshes[i])
        return meshes

    if type == "ADDTRANSFORMATION":
        def getnode(node, name):
            if node.name == name: return node
            for child in node.children:
                n = getnode(child, name)
                if n: return n

        node = getnode(scene.rootnode, obj.name)
        if not node:
            raise AssimpError("Object " + str(obj) + " has no associated node!")
        setattr(obj, "transformation", node.transformation)

def recur_pythonize(node, scene):
    '''
    Recursively call pythonize_assimp on
    nodes tree to apply several post-processing to
    pythonize the assimp datastructures.
    '''
    node.meshes = pythonize_assimp("MESH", node.meshes, scene)
    for mesh in node.meshes:
        mesh.material = scene.materials[mesh.materialindex]
    for cam in scene.cameras:
        pythonize_assimp("ADDTRANSFORMATION", cam, scene)
    for c in node.children:
        recur_pythonize(c, scene)

def load(filename,
         file_type  = None,
         processing = postprocess.aiProcess_Triangulate):
    '''
    Load a model into a scene. On failure throws AssimpError.

    Arguments
    ---------
    filename:   Either a filename or a file object to load model from.
                If a file object is passed, file_type MUST be specified
                Otherwise Assimp has no idea which importer to use.
                This is named 'filename' so as to not break legacy code.
    processing: assimp postprocessing parameters. Verbose keywords are imported
                from postprocessing, and the parameters can be combined bitwise to
                generate the final processing value. Note that the default value will
                triangulate quad faces. Example of generating other possible values:
                processing = (pyassimp.postprocess.aiProcess_Triangulate |
                              pyassimp.postprocess.aiProcess_OptimizeMeshes)
    file_type:  string of file extension, such as 'stl'

    Returns
    ---------
    Scene object with model data
    '''

    if hasattr(filename, 'read'):
        # This is the case where a file object has been passed to load.
        # It is calling the following function:
        # const aiScene* aiImportFileFromMemory(const char* pBuffer,
        #                                      unsigned int pLength,
        #                                      unsigned int pFlags,
        #                                      const char* pHint)
        if file_type is None:
            raise AssimpError('File type must be specified when passing file objects!')
        data  = filename.read()
        model = _assimp_lib.load_mem(data,
                                     len(data),
                                     processing,
                                     file_type)
    else:
        # a filename string has been passed
        model = _assimp_lib.load(filename.encode(sys.getfilesystemencoding()), processing)

    if not model:
        raise AssimpError('Could not import file!')
    scene = _init(model.contents)
    recur_pythonize(scene.rootnode, scene)
    return scene

def export(scene,
           filename,
           file_type  = None,
           processing = postprocess.aiProcess_Triangulate):
    '''
    Export a scene. On failure throws AssimpError.

    Arguments
    ---------
    scene: scene to export.
    filename: Filename that the scene should be exported to.
    file_type: string of file exporter to use. For example "collada".
    processing: assimp postprocessing parameters. Verbose keywords are imported
                from postprocessing, and the parameters can be combined bitwise to
                generate the final processing value. Note that the default value will
                triangulate quad faces. Example of generating other possible values:
                processing = (pyassimp.postprocess.aiProcess_Triangulate |
                              pyassimp.postprocess.aiProcess_OptimizeMeshes)

    '''

    exportStatus = _assimp_lib.export(ctypes.pointer(scene), file_type.encode("ascii"), filename.encode(sys.getfilesystemencoding()), processing)

    if exportStatus != 0:
        raise AssimpError('Could not export scene!')

def export_blob(scene,
                file_type = None,
                processing = postprocess.aiProcess_Triangulate):
    '''
    Export a scene and return a blob in the correct format. On failure throws AssimpError.

    Arguments
    ---------
    scene: scene to export.
    file_type: string of file exporter to use. For example "collada".
    processing: assimp postprocessing parameters. Verbose keywords are imported
                from postprocessing, and the parameters can be combined bitwise to
                generate the final processing value. Note that the default value will
                triangulate quad faces. Example of generating other possible values:
                processing = (pyassimp.postprocess.aiProcess_Triangulate |
                              pyassimp.postprocess.aiProcess_OptimizeMeshes)
    Returns
    ---------
    Pointer to structs.ExportDataBlob
    '''
    exportBlobPtr = _assimp_lib.export_blob(ctypes.pointer(scene), file_type.encode("ascii"), processing)

    if exportBlobPtr == 0:
        raise AssimpError('Could not export scene to blob!')
    return exportBlobPtr

def release(scene):
    _assimp_lib.release(ctypes.pointer(scene))

def _finalize_texture(tex, target):
    setattr(target, "achformathint", tex.achFormatHint)
    if numpy:
        data = numpy.array([make_tuple(getattr(tex, "pcData")[i]) for i in range(tex.mWidth * tex.mHeight)])
    else:
        data = [make_tuple(getattr(tex, "pcData")[i]) for i in range(tex.mWidth * tex.mHeight)]
    setattr(target, "data", data)

def _finalize_mesh(mesh, target):
    """ Building of meshes is a bit specific.

    We override here the various datasets that can
    not be process as regular fields.

    For instance, the length of the normals array is
    mNumVertices (no mNumNormals is available)
    """
    nb_vertices = getattr(mesh, "mNumVertices")

    def fill(name):
        mAttr = getattr(mesh, name)
        if numpy:
            if mAttr:
                data = numpy.array([make_tuple(getattr(mesh, name)[i]) for i in range(nb_vertices)], dtype=numpy.float32)
                setattr(target, name[1:].lower(), data)
            else:
                setattr(target, name[1:].lower(), numpy.array([], dtype="float32"))
        else:
            if mAttr:
                data = [make_tuple(getattr(mesh, name)[i]) for i in range(nb_vertices)]
                setattr(target, name[1:].lower(), data)
            else:
                setattr(target, name[1:].lower(), [])

    def fillarray(name):
        mAttr = getattr(mesh, name)

        data = []
        for index, mSubAttr in enumerate(mAttr):
            if mSubAttr:
                data.append([make_tuple(getattr(mesh, name)[index][i]) for i in range(nb_vertices)])

        if numpy:
            setattr(target, name[1:].lower(), numpy.array(data, dtype=numpy.float32))
        else:
            setattr(target, name[1:].lower(), data)

    fill("mNormals")
    fill("mTangents")
    fill("mBitangents")

    fillarray("mColors")
    fillarray("mTextureCoords")

    # prepare faces
    if numpy:
        faces = numpy.array([f.indices for f in target.faces], dtype=numpy.int32)
    else:
        faces = [f.indices for f in target.faces]
    setattr(target, 'faces', faces)

def _init_metadata_entry(entry):
    entry.type = entry.mType
    if entry.type == structs.MetadataEntry.AI_BOOL:
        entry.data = ctypes.cast(entry.mData, ctypes.POINTER(ctypes.c_bool)).contents.value
    elif entry.type == structs.MetadataEntry.AI_INT32:
        entry.data = ctypes.cast(entry.mData, ctypes.POINTER(ctypes.c_int32)).contents.value
    elif entry.type == structs.MetadataEntry.AI_UINT64:
        entry.data = ctypes.cast(entry.mData, ctypes.POINTER(ctypes.c_uint64)).contents.value
    elif entry.type == structs.MetadataEntry.AI_FLOAT:
        entry.data = ctypes.cast(entry.mData, ctypes.POINTER(ctypes.c_float)).contents.value
    elif entry.type == structs.MetadataEntry.AI_DOUBLE:
        entry.data = ctypes.cast(entry.mData, ctypes.POINTER(ctypes.c_double)).contents.value
    elif entry.type == structs.MetadataEntry.AI_AISTRING:
        assimp_string = ctypes.cast(entry.mData, ctypes.POINTER(structs.String)).contents
        entry.data = _convert_assimp_string(assimp_string)
    elif entry.type == structs.MetadataEntry.AI_AIVECTOR3D:
        assimp_vector = ctypes.cast(entry.mData, ctypes.POINTER(structs.Vector3D)).contents
        entry.data = make_tuple(assimp_vector)

    return entry

def _finalize_metadata(metadata, target):
    """ Building the metadata object is a bit specific.

    Firstly, there are two separate arrays: one with metadata keys and one
    with metadata values, and there are no corresponding mNum* attributes,
    so the C arrays are not converted to Python arrays using the generic
    code in the _init function.

    Secondly, a metadata entry value has to be cast according to declared
    metadata entry type.
    """
    length = metadata.mNumProperties
    setattr(target, 'keys', [str(_convert_assimp_string(metadata.mKeys[i])) for i in range(length)])
    setattr(target, 'values', [_init_metadata_entry(metadata.mValues[i]) for i in range(length)])

class PropertyGetter(dict):
    def __getitem__(self, key):
        semantic = 0
        if isinstance(key, tuple):
            key, semantic = key

        return dict.__getitem__(self, (key, semantic))

    def keys(self):
        for k in dict.keys(self):
            yield k[0]

    def __iter__(self):
        return self.keys()

    def items(self):
        for k, v in dict.items(self):
            yield k[0], v


def _get_properties(properties, length):
    """
    Convenience Function to get the material properties as a dict
    and values in a python format.
    """
    result = {}
    #read all properties
    for p in [properties[i] for i in range(length)]:
        #the name
        p = p.contents
        key = str(_convert_assimp_string(p.mKey))
        key = (key.split('.')[1], p.mSemantic)

        #the data
        if p.mType == 1:
            arr = ctypes.cast(p.mData,
                              ctypes.POINTER(ctypes.c_float * int(p.mDataLength/ctypes.sizeof(ctypes.c_float)))
                              ).contents
            value = [x for x in arr]
        elif p.mType == 3: #string can't be an array
            value = _convert_assimp_string(ctypes.cast(p.mData, ctypes.POINTER(structs.MaterialPropertyString)).contents)

        elif p.mType == 4:
            arr = ctypes.cast(p.mData,
                              ctypes.POINTER(ctypes.c_int * int(p.mDataLength/ctypes.sizeof(ctypes.c_int)))
                              ).contents
            value = [x for x in arr]
        else:
            value = p.mData[:p.mDataLength]

        if len(value) == 1:
            [value] = value

        result[key] = value

    return PropertyGetter(result)

def decompose_matrix(matrix):
    if not isinstance(matrix, structs.Matrix4x4):
        raise AssimpError("pyassimp.decompose_matrix failed: Not a Matrix4x4!")

    scaling = structs.Vector3D()
    rotation = structs.Quaternion()
    position = structs.Vector3D()

    _assimp_lib.dll.aiDecomposeMatrix(ctypes.pointer(matrix),
                                      ctypes.byref(scaling),
                                      ctypes.byref(rotation),
                                      ctypes.byref(position))
    return scaling._init(), rotation._init(), position._init()
    
