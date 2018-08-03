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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;


/**
 * A mesh represents a geometry or model with a single material.
 * <p>
 * 
 * <h3>Data</h3>
 * Meshes usually consist of a number of vertices and a series of faces
 * referencing the vertices. In addition there might be a series of bones, each
 * of them addressing a number of vertices with a certain weight. Vertex data is
 * presented in channels with each channel containing a single per-vertex
 * information such as a set of texture coordinates or a normal vector.<p>
 * 
 * Faces consist of one or more references to vertices, called vertex indices.
 * The {@link #getPrimitiveTypes()} method can be used to check what
 * face types are present in the mesh. Note that a single mesh can possess
 * faces of different types. The number of indices used by a specific face can
 * be retrieved with the {@link #getFaceNumIndices(int)} method.
 * 
 * 
 * <h3>API for vertex and face data</h3>
 * The jassimp interface for accessing vertex and face data is not a one-to-one
 * mapping of the c/c++ interface. The c/c++ interface uses an object-oriented 
 * approach to represent data, which provides a considerable 
 * overhead using a naive java based realization (cache locality would be 
 * unpredictable and most likely bad, bulk data transfer would be impossible).
 * <p>
 * 
 * The jassimp interface uses flat byte buffers to store vertex and face data.
 * This data can be accessed through three APIs:
 * <ul>
 *   <li><b>Buffer API:</b> the <code>getXXXBuffer()</code> methods return 
 *     raw data buffers.
 *   <li><b>Direct API:</b> the <code>getXXX()</code> methods allow reading
 *     and writing of individual data values.
 *   <li><b>Wrapped API:</b> the <code>getWrappedXXX()</code> methods provide
 *     an object oriented view on the data.
 * </ul>
 * 
 * The Buffer API is optimized for use in conjunction with rendering APIs
 * such as LWJGL. The returned buffers are guaranteed to have native byte order
 * and to be direct byte buffers. They can be passed directly to LWJGL
 * methods, e.g., to fill VBOs with data. Each invocation of a
 * <code>getXXXBuffer()</code> method will return a new view of the internal
 * buffer, i.e., if is safe to use the relative byte buffer operations.
 * The Buffer API provides the best performance of all three APIs, especially 
 * if large data volumes have to be processed.<p>
 * 
 * The Direct API provides an easy to use interface for reading and writing
 * individual data values. Its performance is comparable to the Buffer API's
 * performance for these operations. The main difference to the Buffer API is
 * the missing support for bulk operations. If you intend to retrieve or modify
 * large subsets of the raw data consider using the Buffer API, especially
 * if the subsets are contiguous.
 * <p>
 * 
 * The Wrapped API offers an object oriented interface for accessing
 * and modifying mesh data. As the name implies, this interface is realized 
 * through wrapper objects that provide a view on the raw data. For each
 * invocation of a <code>getWrappedXXX()</code> method, a new wrapper object
 * is created. Iterating over mesh data via this interface will create many
 * short-lived wrapper objects which -depending on usage and virtual machine-
 * may cause considerable garbage collection overhead. The Wrapped API provides
 * the worst performance of all three APIs, which may nevertheless still be
 * good enough to warrant its usage. See {@link AiWrapperProvider} for more
 * details on wrappers.
 * 
 * 
 * <h3>API for bones</h3>
 * As there is no standardized way for doing skinning in different graphics
 * engines, bones are not represented as flat buffers but as object structure. 
 * Users of this library should convert this structure to the format required 
 * by the specific graphics engine. 
 * 
 * 
 * <h3>Changing Data</h3>
 * This class is designed to be mutable, i.e., the returned objects and buffers 
 * may be modified. It is not possible to add/remove vertices as this would 
 * require reallocation of the data buffers. Wrapped objects may or may not
 * propagate changes to the underlying data buffers. Consult the documentation
 * of your wrapper provider for details. The built in wrappers will propagate
 * changes.
 * <p>
 * Modification of face data is theoretically possible by modifying the face
 * buffer and the faceOffset buffer however it is strongly disadvised to do so
 * because it might break all algorithms that depend on the internal consistency
 * of these two data structures.
 */
public final class AiMesh {
    /**
     * Number of bytes per float value.
     */
    private final int SIZEOF_FLOAT = Jassimp.NATIVE_FLOAT_SIZE;
        
    /**
     * Number of bytes per int value.
     */
    private final int SIZEOF_INT = Jassimp.NATIVE_INT_SIZE;

    /**
     * Size of an AiVector3D in the native world.
     */
    private final int SIZEOF_V3D = Jassimp.NATIVE_AIVEKTOR3D_SIZE;
    
    
    /**
     * This class is instantiated via JNI, no accessible constructor.
     */
    private AiMesh() {
        /* nothing to do */
    }
    
    
    /**
     * Returns the primitive types used by this mesh.
     * 
     * @return a set of primitive types used by this mesh
     */
    public Set<AiPrimitiveType> getPrimitiveTypes() {
        return m_primitiveTypes;
    }
    
    
    /**
     * Tells whether the mesh is a pure triangle mesh, i.e., contains only
     * triangular faces.<p>
     * 
     * To automatically triangulate meshes the 
     * {@link AiPostProcessSteps#TRIANGULATE} post processing option can be 
     * used when loading the scene
     * 
     * @return true if the mesh is a pure triangle mesh, false otherwise
     */
    public boolean isPureTriangle() {
        return m_primitiveTypes.contains(AiPrimitiveType.TRIANGLE) &&
                m_primitiveTypes.size() == 1;
    }
    
    
    /**
     * Tells whether the mesh has vertex positions.<p>
     * 
     * Meshes almost always contain position data
     * 
     * @return true if positions are available
     */
    public boolean hasPositions() {
        return m_vertices != null;
    }
    
    
    /**
     * Tells whether the mesh has faces.<p>
     * 
     * Meshes almost always contain faces
     * 
     * @return true if faces are available
     */
    public boolean hasFaces() {
        return m_faces != null;
    }
    
    
    /**
     * Tells whether the mesh has normals.
     * 
     * @return true if normals are available
     */
    public boolean hasNormals() {
        return m_normals != null;
    }
    
    
    /**
     * Tells whether the mesh has tangents and bitangents.<p>
     * 
     * It is not possible that it contains tangents and no bitangents (or the 
     * other way round). The existence of one of them implies that the second 
     * is there, too.
     * 
     * @return true if tangents and bitangents are available
     */
    public boolean hasTangentsAndBitangents() {
        return m_tangents != null && m_tangents != null;
    }
    
    
    /**
     * Tells whether the mesh has a vertex color set.
     * 
     * @param colorset index of the color set
     * @return true if colors are available
     */
    public boolean hasColors(int colorset) {
        return m_colorsets[colorset] != null;
    }
    
    
    /**
     * Tells whether the mesh has any vertex colors.<p>
     * 
     * Use {@link #hasColors(int)} to check which color sets are 
     * available.
     * 
     * @return true if any colors are available
     */
    public boolean hasVertexColors() {
        for (ByteBuffer buf : m_colorsets) {
            if (buf != null) {
                return true;
            }
        }
        
        return false;
    }
    
    
    /**
     * Tells whether the mesh has a texture coordinate set.
     * 
     * @param coords index of the texture coordinate set
     * @return true if texture coordinates are available
     */
    public boolean hasTexCoords(int coords) {
        return m_texcoords[coords] != null;
    }
    
    
    /**
     * Tells whether the mesh has any texture coordinate sets.<p>
     * 
     * Use {@link #hasTexCoords(int)} to check which texture coordinate 
     * sets are available
     * 
     * @return true if any texture coordinates are available
     */
    public boolean hasTexCoords() {
        for (ByteBuffer buf : m_texcoords) {
            if (buf != null) {
                return true;
            }
        }
        
        return false;
    }
    
    
    /**
     * Tells whether the mesh has bones.
     * 
     * @return true if bones are available
     */
    public boolean hasBones() {
        return !m_bones.isEmpty();
    }
    
    
    /**
     * Returns the bones of this mesh.
     * 
     * @return a list of bones
     */
    public List<AiBone> getBones() {
        return m_bones;
    }
    
    
    /**
     * Returns the number of vertices in this mesh.
     * 
     * @return the number of vertices.
     */
    public int getNumVertices() {
        return m_numVertices;
    }
    
    
    /**
     * Returns the number of faces in the mesh.
     * 
     * @return the number of faces
     */
    public int getNumFaces() {
        return m_numFaces;
    }
    
    
    /**
     * Returns the number of vertex indices for a single face.
     * 
     * @param face the face
     * @return the number of indices
     */
    public int getFaceNumIndices(int face) {
        if (null == m_faceOffsets) {
            if (face >= m_numFaces || face < 0) {
                throw new IndexOutOfBoundsException("Index: " + face + 
                        ", Size: " + m_numFaces);
            }
            return 3;
        }
        else {
            /* 
             * no need to perform bound checks here as the array access will
             * throw IndexOutOfBoundsExceptions if the index is invalid
             */
            
            if (face == m_numFaces - 1) {
                return m_faces.capacity() / 4 - m_faceOffsets.getInt(face * 4);
            }
            
            return m_faceOffsets.getInt((face + 1) * 4) - 
                    m_faceOffsets.getInt(face * 4);
        }
    }
    
    
    /**
     * Returns the number of UV components for a texture coordinate set.<p>
     * 
     * Possible values range from 1 to 3 (1D to 3D texture coordinates)
     * 
     * @param coords the coordinate set
     * @return the number of components
     */
    public int getNumUVComponents(int coords) {
        return m_numUVComponents[coords];
    }
    
    
    /** 
     * Returns the material used by this mesh.<p>
     *  
     * A mesh does use only a single material. If an imported model uses
     * multiple materials, the import splits up the mesh. Use this value 
     * as index into the scene's material list.
     * 
     * @return the material index
     */
    public int getMaterialIndex() {
        return m_materialIndex;
    }
    
    
    /**
     * Returns the name of the mesh.<p>
     * 
     * Not all meshes have a name, if no name is set an empty string is 
     * returned.
     * 
     * @return the name or an empty string if no name is set
     */
    public String getName() {
        return m_name;
    }
    
    
    // CHECKSTYLE:OFF
    @Override
    public String toString() {
        StringBuilder buf = new StringBuilder();
        buf.append("Mesh(").append(m_numVertices).append(" vertices, ").
            append(m_numFaces).append(" faces");
        
        if (hasNormals()) {
            buf.append(", normals");
        }
        if (hasTangentsAndBitangents()) {
            buf.append(", (bi-)tangents");
        }
        if (hasVertexColors()) {
            buf.append(", colors");
        }
        if (hasTexCoords()) {
            buf.append(", texCoords");
        }
        
        buf.append(")");
        return buf.toString();
    }
    // CHECKSTYLE:ON
    
    
    // {{ Buffer API
    /**
     * Returns a buffer containing vertex positions.<p>
     * 
     * A vertex position consists of a triple of floats, the buffer will 
     * therefore contain <code>3 * getNumVertices()</code> floats
     * 
     * @return a native-order direct buffer, or null if no data is available
     */
    public FloatBuffer getPositionBuffer() {
        if (m_vertices == null) {
            return null;
        }
        
        return m_vertices.asFloatBuffer();
    }
    
    
    /**
     * Returns a buffer containing face data.<p>
     * 
     * You should use the {@link #getIndexBuffer()} method if you are 
     * interested in getting an index buffer used by graphics APIs such as 
     * LWJGL.<p>
     * 
     * The buffer contains all vertex indices from all faces as a flat list. If
     * the mesh is a pure triangle mesh, the buffer returned by this method is
     * identical to the buffer returned by {@link #getIndexBuffer()}. For other
     * meshes, the {@link #getFaceOffsets()} method can be used to retrieve
     * an index structure that allows addressing individual faces in the list. 
     * 
     * @return a native-order direct buffer, or null if no data is available
     */
    public IntBuffer getFaceBuffer() {
        if (m_faces == null) {
            return null;
        }
        
        return m_faces.asIntBuffer();
    }
    
    
    /**
     * Returns an index structure for the buffer returned by 
     * {@link #getFaceBuffer()}.<p>
     * 
     * You should use the {@link #getIndexBuffer()} method if you are 
     * interested in getting an index buffer used by graphics APIs such as 
     * LWJGL.<p>
     * 
     * The returned buffer contains one integer entry for each face. This entry 
     * specifies the offset at which the face's data is located inside the
     * face buffer. The difference between two subsequent entries can be used
     * to determine how many vertices belong to a given face (the last face
     * contains all entries between the offset and the end of the face buffer).
     * 
     * @return a native-order direct buffer, or null if no data is available
     */
    public IntBuffer getFaceOffsets() {
        if (m_faceOffsets == null) {
            return null;
        }
        
        return m_faceOffsets.asIntBuffer();
    }
    
    
    
    /**
     * Returns a buffer containing vertex indices for the mesh's faces.<p>
     * 
     * This method may only be called on pure triangle meshes, i.e., meshes
     * containing only triangles. The {@link #isPureTriangle()} method can be 
     * used to check whether this is the case.<p>
     * 
     * Indices are stored as integers, the buffer will therefore contain 
     * <code>3 * getNumVertices()</code> integers (3 indices per triangle)
     * 
     * @return a native-order direct buffer
     * @throws UnsupportedOperationException
     *             if the mesh is not a pure triangle mesh
     */
    public IntBuffer getIndexBuffer() {
        if (!isPureTriangle()) {
            throw new UnsupportedOperationException(
                    "mesh is not a pure triangle mesh");
        }
        
        return getFaceBuffer();
    }
    
    
    /**
     * Returns a buffer containing normals.<p>
     * 
     * A normal consists of a triple of floats, the buffer will 
     * therefore contain <code>3 * getNumVertices()</code> floats
     * 
     * @return a native-order direct buffer
     */
    public FloatBuffer getNormalBuffer() {
        if (m_normals == null) {
            return null;
        }
        
        return m_normals.asFloatBuffer();
    }
    
    
    /**
     * Returns a buffer containing tangents.<p>
     * 
     * A tangent consists of a triple of floats, the buffer will 
     * therefore contain <code>3 * getNumVertices()</code> floats
     * 
     * @return a native-order direct buffer
     */
    public FloatBuffer getTangentBuffer() {
        if (m_tangents == null) {
            return null;
        }
        
        return m_tangents.asFloatBuffer();
    }
    
    
    /**
     * Returns a buffer containing bitangents.<p>
     * 
     * A bitangent consists of a triple of floats, the buffer will 
     * therefore contain <code>3 * getNumVertices()</code> floats
     * 
     * @return a native-order direct buffer
     */
    public FloatBuffer getBitangentBuffer() {
        if (m_bitangents == null) {
            return null;
        }
        
        return m_bitangents.asFloatBuffer();
    }
    
    
    /**
     * Returns a buffer containing vertex colors for a color set.<p>
     * 
     * A vertex color consists of 4 floats (red, green, blue and alpha), the 
     * buffer will therefore contain <code>4 * getNumVertices()</code> floats
     * 
     * @param colorset the color set
     * 
     * @return a native-order direct buffer, or null if no data is available
     */
    public FloatBuffer getColorBuffer(int colorset) {
        if (m_colorsets[colorset] == null) {
            return null;
        }
        
        return m_colorsets[colorset].asFloatBuffer();
    }
    
    
    /**
     * Returns a buffer containing coordinates for a texture coordinate set.<p>
     * 
     * A texture coordinate consists of up to 3 floats (u, v, w). The actual
     * number can be queried via {@link #getNumUVComponents(int)}. The 
     * buffer will contain 
     * <code>getNumUVComponents(coords) * getNumVertices()</code> floats
     * 
     * @param coords the texture coordinate set
     * 
     * @return a native-order direct buffer, or null if no data is available
     */
    public FloatBuffer getTexCoordBuffer(int coords) {
        if (m_texcoords[coords] == null) {
            return null;
        }
        
        return m_texcoords[coords].asFloatBuffer();
    }
    // }}
    
    
    // {{ Direct API
    /**
     * Returns the x-coordinate of a vertex position.
     * 
     * @param vertex the vertex index
     * @return the x coordinate
     */
    public float getPositionX(int vertex) {
        if (!hasPositions()) {
            throw new IllegalStateException("mesh has no positions");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_vertices.getFloat(vertex * 3 * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the y-coordinate of a vertex position.
     * 
     * @param vertex the vertex index
     * @return the y coordinate
     */
    public float getPositionY(int vertex) {
        if (!hasPositions()) {
            throw new IllegalStateException("mesh has no positions");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_vertices.getFloat((vertex * 3 + 1) * SIZEOF_FLOAT);
    }
    
    /**
     * Returns the z-coordinate of a vertex position.
     * 
     * @param vertex the vertex index
     * @return the z coordinate
     */
    public float getPositionZ(int vertex) {
        if (!hasPositions()) {
            throw new IllegalStateException("mesh has no positions");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_vertices.getFloat((vertex * 3 + 2) * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns a vertex reference from a face.<p>
     * 
     * A face contains <code>getFaceNumIndices(face)</code> vertex references.
     * This method returns the n'th of these. The returned index can be passed
     * directly to the vertex oriented methods, such as 
     * <code>getPosition()</code> etc.
     * 
     * @param face the face
     * @param n the reference
     * @return a vertex index
     */
    public int getFaceVertex(int face, int n) {
        if (!hasFaces()) {
            throw new IllegalStateException("mesh has no faces");
        }
        
        if (face >= m_numFaces || face < 0) {
            throw new IndexOutOfBoundsException("Index: " + face + ", Size: " +
                    m_numFaces);
        }
        if (n >= getFaceNumIndices(face) || n < 0) {
            throw new IndexOutOfBoundsException("Index: " + n + ", Size: " +
                    getFaceNumIndices(face));
        }
        
        int faceOffset = 0;
        if (m_faceOffsets == null) {
            faceOffset = 3 * face * SIZEOF_INT;
        }
        else {
            faceOffset = m_faceOffsets.getInt(face * SIZEOF_INT) * SIZEOF_INT;
        }
        
        return m_faces.getInt(faceOffset + n * SIZEOF_INT);
    }
    
    
    /**
     * Returns the x-coordinate of a vertex normal.
     * 
     * @param vertex the vertex index
     * @return the x coordinate
     */
    public float getNormalX(int vertex) {
        if (!hasNormals()) {
            throw new IllegalStateException("mesh has no normals");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_normals.getFloat(vertex * 3 * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the y-coordinate of a vertex normal.
     * 
     * @param vertex the vertex index
     * @return the y coordinate
     */
    public float getNormalY(int vertex) {
        if (!hasNormals()) {
            throw new IllegalStateException("mesh has no normals");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_normals.getFloat((vertex * 3 + 1) * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the z-coordinate of a vertex normal.
     * 
     * @param vertex the vertex index
     * @return the z coordinate
     */
    public float getNormalZ(int vertex) {
        if (!hasNormals()) {
            throw new IllegalStateException("mesh has no normals");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_normals.getFloat((vertex * 3 + 2) * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the x-coordinate of a vertex tangent.
     * 
     * @param vertex the vertex index
     * @return the x coordinate
     */
    public float getTangentX(int vertex) {
        if (!hasTangentsAndBitangents()) {
            throw new IllegalStateException("mesh has no tangents");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_tangents.getFloat(vertex * 3 * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the y-coordinate of a vertex bitangent.
     * 
     * @param vertex the vertex index
     * @return the y coordinate
     */
    public float getTangentY(int vertex) {
        if (!hasTangentsAndBitangents()) {
            throw new IllegalStateException("mesh has no bitangents");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_tangents.getFloat((vertex * 3 + 1) * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the z-coordinate of a vertex tangent.
     * 
     * @param vertex the vertex index
     * @return the z coordinate
     */
    public float getTangentZ(int vertex) {
        if (!hasTangentsAndBitangents()) {
            throw new IllegalStateException("mesh has no tangents");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_tangents.getFloat((vertex * 3 + 2) * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the x-coordinate of a vertex tangent.
     * 
     * @param vertex the vertex index
     * @return the x coordinate
     */
    public float getBitangentX(int vertex) {
        if (!hasTangentsAndBitangents()) {
            throw new IllegalStateException("mesh has no bitangents");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_bitangents.getFloat(vertex * 3 * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the y-coordinate of a vertex tangent.
     * 
     * @param vertex the vertex index
     * @return the y coordinate
     */
    public float getBitangentY(int vertex) {
        if (!hasTangentsAndBitangents()) {
            throw new IllegalStateException("mesh has no bitangents");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_bitangents.getFloat((vertex * 3 + 1) * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the z-coordinate of a vertex tangent.
     * 
     * @param vertex the vertex index
     * @return the z coordinate
     */
    public float getBitangentZ(int vertex) {
        if (!hasTangentsAndBitangents()) {
            throw new IllegalStateException("mesh has no bitangents");
        }
        
        checkVertexIndexBounds(vertex);
        
        return m_bitangents.getFloat((vertex * 3 + 2) * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the red color component of a color from a vertex color set.
     * 
     * @param vertex the vertex index
     * @param colorset the color set
     * @return the red color component
     */
    public float getColorR(int vertex, int colorset) {
        if (!hasColors(colorset)) {
            throw new IllegalStateException("mesh has no colorset " + colorset);
        }
        
        checkVertexIndexBounds(vertex);
        /* bound checks for colorset are done by java for us */
        
        return m_colorsets[colorset].getFloat(vertex * 4 * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the green color component of a color from a vertex color set.
     * 
     * @param vertex the vertex index
     * @param colorset the color set
     * @return the green color component
     */
    public float getColorG(int vertex, int colorset) {
        if (!hasColors(colorset)) {
            throw new IllegalStateException("mesh has no colorset " + colorset);
        }
        
        checkVertexIndexBounds(vertex);
        /* bound checks for colorset are done by java for us */
        
        return m_colorsets[colorset].getFloat((vertex * 4 + 1) * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the blue color component of a color from a vertex color set.
     * 
     * @param vertex the vertex index
     * @param colorset the color set
     * @return the blue color component
     */
    public float getColorB(int vertex, int colorset) {
        if (!hasColors(colorset)) {
            throw new IllegalStateException("mesh has no colorset " + colorset);
        }
        
        checkVertexIndexBounds(vertex);
        /* bound checks for colorset are done by java for us */
        
        return m_colorsets[colorset].getFloat((vertex * 4 + 2) * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the alpha color component of a color from a vertex color set.
     * 
     * @param vertex the vertex index
     * @param colorset the color set
     * @return the alpha color component
     */
    public float getColorA(int vertex, int colorset) {
        if (!hasColors(colorset)) {
            throw new IllegalStateException("mesh has no colorset " + colorset);
        }
        
        checkVertexIndexBounds(vertex);
        /* bound checks for colorset are done by java for us */
        
        return m_colorsets[colorset].getFloat((vertex * 4 + 3) * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the u component of a coordinate from a texture coordinate set.
     * 
     * @param vertex the vertex index
     * @param coords the texture coordinate set
     * @return the u component
     */
    public float getTexCoordU(int vertex, int coords) {
        if (!hasTexCoords(coords)) {
            throw new IllegalStateException(
                    "mesh has no texture coordinate set " + coords);
        }
        
        checkVertexIndexBounds(vertex);
        /* bound checks for coords are done by java for us */
        
        return m_texcoords[coords].getFloat(
                vertex * m_numUVComponents[coords] * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the v component of a coordinate from a texture coordinate set.<p>
     * 
     * This method may only be called on 2- or 3-dimensional coordinate sets.
     * Call <code>getNumUVComponents(coords)</code> to determine how may 
     * coordinate components are available.
     * 
     * @param vertex the vertex index
     * @param coords the texture coordinate set
     * @return the v component
     */
    public float getTexCoordV(int vertex, int coords) {
        if (!hasTexCoords(coords)) {
            throw new IllegalStateException(
                    "mesh has no texture coordinate set " + coords);
        }
        
        checkVertexIndexBounds(vertex);
        
        /* bound checks for coords are done by java for us */
        
        if (getNumUVComponents(coords) < 2) {
            throw new IllegalArgumentException("coordinate set " + coords + 
                    " does not contain 2D texture coordinates");
        }
        
        return m_texcoords[coords].getFloat(
                (vertex * m_numUVComponents[coords] + 1) * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the w component of a coordinate from a texture coordinate set.<p>
     * 
     * This method may only be called on 3-dimensional coordinate sets.
     * Call <code>getNumUVComponents(coords)</code> to determine how may 
     * coordinate components are available.
     * 
     * @param vertex the vertex index
     * @param coords the texture coordinate set
     * @return the w component
     */
    public float getTexCoordW(int vertex, int coords) {
        if (!hasTexCoords(coords)) {
            throw new IllegalStateException(
                    "mesh has no texture coordinate set " + coords);
        }
        
        checkVertexIndexBounds(vertex);
        
        /* bound checks for coords are done by java for us */
        
        if (getNumUVComponents(coords) < 3) {
            throw new IllegalArgumentException("coordinate set " + coords + 
                    " does not contain 3D texture coordinates");
        }
        
        return m_texcoords[coords].getFloat(
                (vertex * m_numUVComponents[coords] + 1) * SIZEOF_FLOAT);
    }
    // }}
    
    
    // {{ Wrapped API
    /**
     * Returns the vertex position as 3-dimensional vector.<p>
     *
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param vertex the vertex index
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the position wrapped as object
     */
    public <V3, M4, C, N, Q> V3 getWrappedPosition(int vertex, 
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        if (!hasPositions()) {
            throw new IllegalStateException("mesh has no positions");
        }
        
        checkVertexIndexBounds(vertex);
        
        return wrapperProvider.wrapVector3f(m_vertices, 
                vertex * 3 * SIZEOF_FLOAT, 3);
    }
    
    
    /**
     * Returns the vertex normal as 3-dimensional vector.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     *  
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param vertex the vertex index
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the normal wrapped as object
     */
    public <V3, M4, C, N, Q> V3 getWrappedNormal(int vertex,
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        if (!hasNormals()) {
            throw new IllegalStateException("mesh has no positions");
        }
        
        checkVertexIndexBounds(vertex);
        
        return wrapperProvider.wrapVector3f(m_normals, 
                vertex * 3 * SIZEOF_FLOAT, 3);
    }
    
    
    /**
     * Returns the vertex tangent as 3-dimensional vector.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param vertex the vertex index
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the tangent wrapped as object
     */
    public <V3, M4, C, N, Q> V3 getWrappedTangent(int vertex,
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        if (!hasTangentsAndBitangents()) {
            throw new IllegalStateException("mesh has no tangents");
        }
        
        checkVertexIndexBounds(vertex);
        
        return wrapperProvider.wrapVector3f(m_tangents, 
                vertex * 3 * SIZEOF_FLOAT, 3);
    }
    
    
    /**
     * Returns the vertex bitangent as 3-dimensional vector.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param vertex the vertex index
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the bitangent wrapped as object
     */
    public <V3, M4, C, N, Q> V3 getWrappedBitangent(int vertex,
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        if (!hasTangentsAndBitangents()) {
            throw new IllegalStateException("mesh has no bitangents");
        }
        
        checkVertexIndexBounds(vertex);
        
        return wrapperProvider.wrapVector3f(m_bitangents, 
                vertex * 3 * SIZEOF_FLOAT, 3);
    }
    
    
    /**
     * Returns the vertex color.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiColor}.
     * 
     * @param vertex the vertex index
     * @param colorset the color set
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the vertex color wrapped as object
     */
    public <V3, M4, C, N, Q> C getWrappedColor(int vertex, int colorset,
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        if (!hasColors(colorset)) {
            throw new IllegalStateException("mesh has no colorset " + colorset);
        }
        
        checkVertexIndexBounds(vertex);
        
        return wrapperProvider.wrapColor(
                m_colorsets[colorset], vertex * 4 * SIZEOF_FLOAT);
    }
    
    
    /**
     * Returns the texture coordinates as n-dimensional vector.<p>
     *
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built-in behavior is to return a {@link AiVector}.
     * 
     * @param vertex the vertex index
     * @param coords the texture coordinate set
     * @param wrapperProvider the wrapper provider (used for type inference)
     * @return the texture coordinates wrapped as object
     */
    public <V3, M4, C, N, Q> V3 getWrappedTexCoords(int vertex, int coords,
            AiWrapperProvider<V3, M4, C, N, Q> wrapperProvider) {
        
        if (!hasTexCoords(coords)) {
            throw new IllegalStateException(
                    "mesh has no texture coordinate set " + coords);
        }
        
        checkVertexIndexBounds(vertex);
        
        return wrapperProvider.wrapVector3f(m_texcoords[coords], 
                vertex * 3 * SIZEOF_FLOAT, getNumUVComponents(coords));
    }
    // }}
    
    
    // {{ Helpers
    /**
     * Throws an exception if the vertex index is not in the allowed range.
     * 
     * @param vertex the index to check
     */
    private void checkVertexIndexBounds(int vertex) {
        if (vertex >= m_numVertices || vertex < 0) {
            throw new IndexOutOfBoundsException("Index: " + vertex + 
                    ", Size: " + m_numVertices);
        }
    }
    // }}
    
    // {{ JNI interface
    /* 
     * Channel constants used by allocate data channel. Do not modify or use
     * as these may change at will  
     */
    // CHECKSTYLE:OFF
    private static final int NORMALS = 0;
    private static final int TANGENTS = 1;
    private static final int BITANGENTS = 2;
    private static final int COLORSET = 3;
    private static final int TEXCOORDS_1D = 4;
    private static final int TEXCOORDS_2D = 5;
    private static final int TEXCOORDS_3D = 6;
    // CHECKSTYLE:ON
    
    
    /**
     * This method is used by JNI. Do not call or modify.<p>
     * 
     * Sets the primitive types enum set
     * 
     * @param types the bitwise or'ed c/c++ aiPrimitiveType enum values
     */
    @SuppressWarnings("unused")
    private void setPrimitiveTypes(int types) {
        AiPrimitiveType.fromRawValue(m_primitiveTypes, types);
    }
    
    
    /**
     * This method is used by JNI. Do not call or modify.<p>
     * 
     * Allocates byte buffers
     * 
     * @param numVertices the number of vertices in the mesh
     * @param numFaces the number of faces in the mesh
     * @param optimizedFaces set true for optimized face representation
     * @param faceBufferSize size of face buffer for non-optimized face
     *              representation
     */
    @SuppressWarnings("unused")
    private void allocateBuffers(int numVertices, int numFaces, 
            boolean optimizedFaces, int faceBufferSize) {
        /* 
         * the allocated buffers are native order direct byte buffers, so they
         * can be passed directly to LWJGL or similar graphics APIs
         */
        
        /* ensure face optimization is possible */
        if (optimizedFaces && !isPureTriangle()) {
            throw new IllegalArgumentException("mesh is not purely triangular");
        }
        
        
        m_numVertices = numVertices;
        m_numFaces = numFaces;
        
        
        /* allocate for each vertex 3 floats */
        if (m_numVertices > 0) {
            m_vertices = ByteBuffer.allocateDirect(numVertices * 3 * 
                    SIZEOF_FLOAT);
            m_vertices.order(ByteOrder.nativeOrder());
        }
        
        
        if (m_numFaces > 0) {
            /* for optimized faces allocate 3 integers per face */ 
            if (optimizedFaces) {
                m_faces = ByteBuffer.allocateDirect(numFaces * 3 * SIZEOF_INT);
                m_faces.order(ByteOrder.nativeOrder());
            }
            /* 
             * for non-optimized faces allocate the passed in buffer size 
             * and allocate the face index structure
             */
            else {
                m_faces = ByteBuffer.allocateDirect(faceBufferSize);
                m_faces.order(ByteOrder.nativeOrder());
                
                m_faceOffsets = ByteBuffer.allocateDirect(numFaces * 
                        SIZEOF_INT);
                m_faceOffsets.order(ByteOrder.nativeOrder());
            }
        }
    }
    
    
    /**
     * This method is used by JNI. Do not call or modify.<p>
     * 
     * Allocates a byte buffer for a vertex data channel
     * 
     * @param channelType the channel type
     * @param channelIndex sub-index, used for types that can have multiple
     *              channels, such as texture coordinates 
     */
    @SuppressWarnings("unused")
    private void allocateDataChannel(int channelType, int channelIndex) {
        switch (channelType) {
        case NORMALS:
            m_normals = ByteBuffer.allocateDirect(
                    m_numVertices * 3 * SIZEOF_FLOAT);
            m_normals.order(ByteOrder.nativeOrder());
            break;
        case TANGENTS:
            m_tangents = ByteBuffer.allocateDirect(
                    m_numVertices * 3 * SIZEOF_FLOAT);
            m_tangents.order(ByteOrder.nativeOrder());
            break;
        case BITANGENTS:
            m_bitangents = ByteBuffer.allocateDirect(
                    m_numVertices * 3 * SIZEOF_FLOAT);
            m_bitangents.order(ByteOrder.nativeOrder());
            break;
        case COLORSET:
            m_colorsets[channelIndex] = ByteBuffer.allocateDirect(
                    m_numVertices * 4 * SIZEOF_FLOAT);
            m_colorsets[channelIndex].order(ByteOrder.nativeOrder());
            break;
        case TEXCOORDS_1D:
            m_numUVComponents[channelIndex] = 1;
            m_texcoords[channelIndex] = ByteBuffer.allocateDirect(
                    m_numVertices * 1 * SIZEOF_FLOAT);
            m_texcoords[channelIndex].order(ByteOrder.nativeOrder());
            break;
        case TEXCOORDS_2D:
            m_numUVComponents[channelIndex] = 2;
            m_texcoords[channelIndex] = ByteBuffer.allocateDirect(
                    m_numVertices * 2 * SIZEOF_FLOAT);
            m_texcoords[channelIndex].order(ByteOrder.nativeOrder());
            break;
        case TEXCOORDS_3D:
            m_numUVComponents[channelIndex] = 3;
            m_texcoords[channelIndex] = ByteBuffer.allocateDirect(
                    m_numVertices * 3 * SIZEOF_FLOAT);
            m_texcoords[channelIndex].order(ByteOrder.nativeOrder());
            break;
        default:
            throw new IllegalArgumentException("unsupported channel type");
        }
    }
    // }}
    
    
    /**
     * The primitive types used by this mesh.
     */
    private final Set<AiPrimitiveType> m_primitiveTypes = 
            EnumSet.noneOf(AiPrimitiveType.class);
    
    
    /**
     * Number of vertices in this mesh.
     */
    private int m_numVertices = 0;
    
    
    /**
     * Number of faces in this mesh.
     */
    private int m_numFaces = 0;
    
    
    /**
     * Material used by this mesh.
     */
    private int m_materialIndex = -1;
    
    
    /**
     * The name of the mesh.
     */
    private String m_name = "";
    
    
    /**
     * Buffer for vertex position data.
     */
    private ByteBuffer m_vertices = null;
    
    
    /**
     * Buffer for faces/ indices.
     */
    private ByteBuffer m_faces = null;
    
    
    /**
     * Index structure for m_faces.<p>
     * 
     * Only used by meshes that are not pure triangular
     */
    private ByteBuffer m_faceOffsets = null;
    
    
    /**
     * Buffer for normals.
     */
    private ByteBuffer m_normals = null;
    
    
    /**
     * Buffer for tangents.
     */
    private ByteBuffer m_tangents = null;
    
    
    /**
     * Buffer for bitangents.
     */
    private ByteBuffer m_bitangents = null;
    
    
    /**
     * Vertex colors.
     */
    private ByteBuffer[] m_colorsets = 
            new ByteBuffer[JassimpConfig.MAX_NUMBER_COLORSETS];
    
    
    /**
     * Number of UV components for each texture coordinate set.
     */
    private int[] m_numUVComponents = new int[JassimpConfig.MAX_NUMBER_TEXCOORDS];
    
    
    /**
     * Texture coordinates.
     */
    private ByteBuffer[] m_texcoords = 
            new ByteBuffer[JassimpConfig.MAX_NUMBER_TEXCOORDS];
    
    
    /**
     * Bones.
     */
    private final List<AiBone> m_bones = new ArrayList<AiBone>();
}
