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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


/**
 * A node in the imported hierarchy.<p>
 *
 * Each node has name, a parent node (except for the root node), 
 * a transformation relative to its parent and possibly several child nodes.
 * Simple file formats don't support hierarchical structures - for these formats
 * the imported scene consists of only a single root node without children.
 */
public final class AiNode {
    /**
     * Constructor.
     * 
     * @param parent the parent node, may be null
     * @param transform the transform matrix
     * @param meshReferences array of mesh references
     * @param name the name of the node
     */
    AiNode(AiNode parent, Object transform, int[] meshReferences, String name) {
        m_parent = parent;
        m_transformationMatrix = transform;
        m_meshReferences = meshReferences;
        m_name = name;
        
        if (null != m_parent) {
            m_parent.addChild(this);
        }
    }
    
    
    /**
     * Returns the name of this node.
     * 
     * @return the name
     */
    public String getName() {
        return m_name;
    }
    
    
    /**
     * Returns the number of child nodes.<p>
     * 
     * This method exists for compatibility reasons with the native assimp API.
     * The returned value is identical to <code>getChildren().size()</code>
     * 
     * @return the number of child nodes
     */
    public int getNumChildren() {
        return getChildren().size();
    }
    
    
    /**
     * Returns a 4x4 matrix that specifies the transformation relative to 
     * the parent node.<p>
     * 
     * This method is part of the wrapped API (see {@link AiWrapperProvider}
     * for details on wrappers).<p>
     * 
     * The built in behavior is to return an {@link AiMatrix4f}.
     * 
     * @param wrapperProvider the wrapper provider (used for type inference)
     * 
     * @return a matrix
     */
    @SuppressWarnings("unchecked")
    public <V3, M4, C, N, Q> M4 getTransform(AiWrapperProvider<V3, M4, C, N, Q> 
            wrapperProvider) {
        
        return (M4) m_transformationMatrix;
    }
    
    
    /**
     * Returns the children of this node.
     * 
     * @return the children, or an empty list if the node has no children
     */
    public List<AiNode> getChildren() {
        return m_children;
    }
    
    
    /**
     * Returns the parent node.
     * 
     * @return the parent, or null of the node has no parent
     */
    public AiNode getParent() {
        return m_parent;
    }
    
    
    /**
     * Searches the node hierarchy below (and including) this node for a node
     * with the specified name.
     * 
     * @param name the name to look for
     * @return the first node with the given name, or null if no such node 
     *              exists
     */
    public AiNode findNode(String name) {
        /* classic recursive depth first search */
        
        if (m_name.equals(name)) {
            return this;
        }
        
        for (AiNode child : m_children) {
            if (null != child.findNode(name)) {
                return child;
            }
        }
        
        return null;
    }
    
    
    /**
     * Returns the number of meshes references by this node.<p>
     * 
     * This method exists for compatibility with the native assimp API.
     * The returned value is identical to <code>getMeshes().length</code>
     * 
     * @return the number of references
     */
    public int getNumMeshes() {
        return m_meshReferences.length;
    }
    
    
    /** 
     * Returns the meshes referenced by this node.<p> 
     * 
     * Each entry is an index into the mesh list stored in {@link AiScene}.
     * 
     * @return an array of indices
     */
    public int[] getMeshes() {
        return m_meshReferences;
    }

    /**
     * Returns the metadata entries for this node.<p>
     *
     * Consult the original Doxygen for importer_notes to
     * see which formats have metadata and what to expect.
     *
     * @return A map of metadata names to entries.
     */
    public Map<String, AiMetadataEntry> getMetadata() {
        return m_metaData;
    }
    
    
    /**
     * Adds a child node.
     * 
     * @param child the child to add
     */
    void addChild(AiNode child) {
        m_children.add(child);
    }
    
    
    /**
     * Name.
     */
    private final String m_name;
    
    
    /**
     * Parent node.
     */
    private final AiNode m_parent;
    
    
    /**
     * Mesh references.
     */
    private final int[] m_meshReferences;
    
    
    /**
     * List of children.
     */
    private final List<AiNode> m_children = new ArrayList<AiNode>();

    /**
     * List of metadata entries.
     */
     private final Map<String, AiMetadataEntry> m_metaData = new HashMap<String, AiMetadataEntry>();
    
    
    /**
     * Buffer for transformation matrix.
     */
    private final Object m_transformationMatrix;
}
