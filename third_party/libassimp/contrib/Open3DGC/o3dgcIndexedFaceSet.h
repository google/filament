/*
Copyright (c) 2013 Khaled Mammou - Advanced Micro Devices, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#pragma once
#ifndef O3DGC_INDEXED_FACE_SET_H
#define O3DGC_INDEXED_FACE_SET_H

#include "o3dgcCommon.h"

namespace o3dgc
{
    template<class T>
    class IndexedFaceSet
    {
    public:    
        //! Constructor.
                                         IndexedFaceSet(void)
                                         {
                                             memset(this, 0, sizeof(IndexedFaceSet));
                                             m_ccw              = true;
                                             m_solid            = true;
                                             m_convex           = true;
                                             m_isTriangularMesh = true;
                                             m_creaseAngle      = 30;
                                         };
        //! Destructor.
                                         ~IndexedFaceSet(void) {};
        
        unsigned long                    GetNCoordIndex() const { return m_nCoordIndex     ;}
        // only coordIndex is supported
        unsigned long                    GetNCoord()           const { return m_nCoord         ;}
        unsigned long                    GetNNormal()          const { return m_nNormal        ;}
        unsigned long                    GetNFloatAttribute(unsigned long a)  const 
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             return m_nFloatAttribute[a];
                                         }
        unsigned long                    GetNIntAttribute(unsigned long a)  const 
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                             return m_nIntAttribute[a];
                                         }
        unsigned long                    GetNumFloatAttributes()  const { return m_numFloatAttributes;}
        unsigned long                    GetNumIntAttributes()    const { return m_numIntAttributes  ;}
        const Real *                     GetCoordMin   () const { return m_coordMin;}
        const Real *                     GetCoordMax   () const { return m_coordMax;}
        const Real *                     GetNormalMin  () const { return m_normalMin;}
        const Real *                     GetNormalMax  () const { return m_normalMax;}
        Real                             GetCoordMin   (int j)  const { return m_coordMin[j]       ;}
        Real                             GetCoordMax   (int j)  const { return m_coordMax[j]       ;}
        Real                             GetNormalMin  (int j)  const { return m_normalMin[j]      ;}
        Real                             GetNormalMax  (int j)  const { return m_normalMax[j]      ;}

        O3DGCIFSFloatAttributeType       GetFloatAttributeType(unsigned long a) const
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             return m_typeFloatAttribute[a];
                                         }
        O3DGCIFSIntAttributeType         GetIntAttributeType(unsigned long a) const
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                             return m_typeIntAttribute[a];
                                         }
        unsigned long                    GetFloatAttributeDim(unsigned long a) const
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             return m_dimFloatAttribute[a];
                                         }
        unsigned long                    GetIntAttributeDim(unsigned long a) const
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                             return m_dimIntAttribute[a];
                                         }
        const Real *                     GetFloatAttributeMin(unsigned long a) const
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             return &(m_minFloatAttribute[a * O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES]);
                                         }
        const Real *                     GetFloatAttributeMax(unsigned long a) const
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             return &(m_maxFloatAttribute[a * O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES]);
                                         }
        Real                             GetFloatAttributeMin(unsigned long a, unsigned long dim) const
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             assert(dim < O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES);
                                             return m_minFloatAttribute[a * O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES + dim];
                                         }
        Real                             GetFloatAttributeMax(unsigned long a, unsigned long dim) const
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             assert(dim < O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES);
                                             return m_maxFloatAttribute[a * O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES + dim];
                                         }
        Real                             GetCreaseAngle()      const { return m_creaseAngle     ;}
        bool                             GetCCW()              const { return m_ccw             ;}
        bool                             GetSolid()            const { return m_solid           ;}
        bool                             GetConvex()           const { return m_convex          ;}
        bool                             GetIsTriangularMesh() const { return m_isTriangularMesh;}
        const unsigned long *            GetIndexBufferID()    const { return m_indexBufferID   ;}
        const T *                        GetCoordIndex()       const { return m_coordIndex;}
        T *                              GetCoordIndex()             { return m_coordIndex;}
        Real *                           GetCoord()            const { return m_coord     ;}
        Real *                           GetNormal()           const { return m_normal    ;}
        Real *                           GetFloatAttribute(unsigned long a)  const
                                         {
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             return m_floatAttribute[a];
                                         }
        long *                           GetIntAttribute(unsigned long a)   const
                                         {
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                             return m_intAttribute[a]  ;
                                         }
        // only coordIndex is supported
        void                             SetNNormalIndex(unsigned long)      {}
        void                             SetNTexCoordIndex(unsigned long)    {}
        void                             SetNFloatAttributeIndex(int, unsigned long) {}
        void                             SetNIntAttributeIndex (int, unsigned long) {}
        // per triangle attributes not supported
        void                             SetNormalPerVertex(bool)   {} 
        void                             SetColorPerVertex(bool)    {}
        void                             SetFloatAttributePerVertex(int, bool){}
        void                             SetIntAttributePerVertex  (int, bool){}
        void                             SetNCoordIndex     (unsigned long nCoordIndex)     { m_nCoordIndex = nCoordIndex;}
        void                             SetNCoord          (unsigned long nCoord)          { m_nCoord      = nCoord     ;}
        void                             SetNNormal         (unsigned long nNormal)         { m_nNormal     = nNormal    ;}
        void                             SetNumFloatAttributes(unsigned long numFloatAttributes) 
                                         { 
                                             assert(numFloatAttributes < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             m_numFloatAttributes = numFloatAttributes;
                                         }
        void                             SetNumIntAttributes  (unsigned long numIntAttributes)
                                         { 
                                             assert(numIntAttributes < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                             m_numIntAttributes = numIntAttributes;
                                         }
        void                             SetCreaseAngle      (Real creaseAngle)      { m_creaseAngle      = creaseAngle     ;}
        void                             SetCCW              (bool ccw)              { m_ccw              = ccw             ;}
        void                             SetSolid            (bool solid)            { m_solid            = solid           ;}
        void                             SetConvex           (bool convex)           { m_convex           = convex          ;}
        void                             SetIsTriangularMesh (bool isTriangularMesh) { m_isTriangularMesh = isTriangularMesh;}
        void                             SetCoordMin        (int j, Real min) { m_coordMin[j]    = min;}
        void                             SetCoordMax        (int j, Real max) { m_coordMax[j]    = max;}
        void                             SetNormalMin       (int j, Real min) { m_normalMin[j]   = min;}
        void                             SetNormalMax       (int j, Real max) { m_normalMax[j]   = max;}
        void                             SetNFloatAttribute(unsigned long a, unsigned long nFloatAttribute) 
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             m_nFloatAttribute[a] = nFloatAttribute;
                                         }
        void                             SetNIntAttribute(unsigned long a, unsigned long nIntAttribute) 
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                             m_nIntAttribute[a] = nIntAttribute;
                                         }
        void                             SetFloatAttributeDim(unsigned long a, unsigned long d)
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             m_dimFloatAttribute[a] = d;
                                         }
        void                             SetIntAttributeDim(unsigned long a, unsigned long d)
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                             m_dimIntAttribute[a] = d;
                                         }
        void                             SetFloatAttributeType(unsigned long a, O3DGCIFSFloatAttributeType t)
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             m_typeFloatAttribute[a] = t;
                                         }
        void                             SetIntAttributeType(unsigned long a, O3DGCIFSIntAttributeType t)
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                             m_typeIntAttribute[a] = t;
                                         }
        void                             SetFloatAttributeMin(unsigned long a, unsigned long dim, Real min) 
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             assert(dim < O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES);
                                             m_minFloatAttribute[a * O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES + dim] = min;
                                         }
        void                             SetFloatAttributeMax(unsigned long a, unsigned long dim, Real max) 
                                         { 
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             assert(dim < O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES);
                                             m_maxFloatAttribute[a * O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES + dim] = max;
                                         }
        void                             SetIndexBufferID  (unsigned long * const indexBufferID) { m_indexBufferID = indexBufferID;}
        void                             SetCoordIndex     (T * const coordIndex)    { m_coordIndex = coordIndex;}
        void                             SetCoord          (Real * const coord     ) { m_coord      = coord    ;}
        void                             SetNormal         (Real * const normal    ) { m_normal     = normal   ;}
        void                             SetFloatAttribute (unsigned long a, Real * const floatAttribute) 
                                         {
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES);
                                             m_floatAttribute[a] = floatAttribute;
                                         }
        void                             SetIntAttribute   (unsigned long a, long * const intAttribute)
                                         {
                                             assert(a < O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES);
                                             m_intAttribute[a] = intAttribute ;
                                         }
        void                             ComputeMinMax(O3DGCSC3DMCQuantizationMode quantMode);

    private:
        // triangles list
        unsigned long                    m_nCoordIndex;
        T *                              m_coordIndex;
        unsigned long *                  m_indexBufferID;
        // coord, normals, texcoord and color
        unsigned long                    m_nCoord;
        unsigned long                    m_nNormal;
        Real                             m_coordMin   [3];
        Real                             m_coordMax   [3];
        Real                             m_normalMin  [3];
        Real                             m_normalMax  [3];
        Real *                           m_coord;
        Real *                           m_normal;
        // other attributes
        unsigned long                    m_numFloatAttributes;
        unsigned long                    m_numIntAttributes;
        O3DGCIFSFloatAttributeType       m_typeFloatAttribute [O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES];
        O3DGCIFSIntAttributeType         m_typeIntAttribute   [O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES  ];
        unsigned long                    m_nFloatAttribute    [O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES];
        unsigned long                    m_nIntAttribute      [O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES  ];
        unsigned long                    m_dimFloatAttribute  [O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES];
        unsigned long                    m_dimIntAttribute    [O3DGC_SC3DMC_MAX_NUM_INT_ATTRIBUTES  ];
        Real                             m_minFloatAttribute  [O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES * O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES];
        Real                             m_maxFloatAttribute  [O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES * O3DGC_SC3DMC_MAX_DIM_ATTRIBUTES];
        Real *                           m_floatAttribute     [O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES];
        long *                           m_intAttribute       [O3DGC_SC3DMC_MAX_NUM_FLOAT_ATTRIBUTES];
        // mesh info                     
        Real                             m_creaseAngle;
        bool                             m_ccw;
        bool                             m_solid;
        bool                             m_convex;
        bool                             m_isTriangularMesh;
    };
}
#include "o3dgcIndexedFaceSet.inl"    // template implementation
#endif // O3DGC_INDEXED_FACE_SET_H

