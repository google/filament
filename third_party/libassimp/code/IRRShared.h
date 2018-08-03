

/** @file  IRRShared.h
  * @brief Shared utilities for the IRR and IRRMESH loaders
  */

#ifndef INCLUDED_AI_IRRSHARED_H
#define INCLUDED_AI_IRRSHARED_H

#include "irrXMLWrapper.h"
#include "BaseImporter.h"
#include <stdint.h>

struct aiMaterial;

namespace Assimp    {


/** @brief Matrix to convert from Assimp to IRR and backwards
 */
extern const aiMatrix4x4 AI_TO_IRR_MATRIX;


// Default: 0 = solid, one texture
#define AI_IRRMESH_MAT_solid_2layer         0x10000

// Transparency flags
#define AI_IRRMESH_MAT_trans_vertex_alpha   0x1
#define AI_IRRMESH_MAT_trans_add            0x2

// Lightmapping flags
#define AI_IRRMESH_MAT_lightmap             0x2
#define AI_IRRMESH_MAT_lightmap_m2          (AI_IRRMESH_MAT_lightmap|0x4)
#define AI_IRRMESH_MAT_lightmap_m4          (AI_IRRMESH_MAT_lightmap|0x8)
#define AI_IRRMESH_MAT_lightmap_light       (AI_IRRMESH_MAT_lightmap|0x10)
#define AI_IRRMESH_MAT_lightmap_light_m2    (AI_IRRMESH_MAT_lightmap|0x20)
#define AI_IRRMESH_MAT_lightmap_light_m4    (AI_IRRMESH_MAT_lightmap|0x40)
#define AI_IRRMESH_MAT_lightmap_add         (AI_IRRMESH_MAT_lightmap|0x80)

// Standard NormalMap (or Parallax map, they're treated equally)
#define AI_IRRMESH_MAT_normalmap_solid      (0x100)

// Normal map combined with vertex alpha
#define AI_IRRMESH_MAT_normalmap_tva    \
    (AI_IRRMESH_MAT_normalmap_solid | AI_IRRMESH_MAT_trans_vertex_alpha)

// Normal map combined with additive transparency
#define AI_IRRMESH_MAT_normalmap_ta     \
    (AI_IRRMESH_MAT_normalmap_solid | AI_IRRMESH_MAT_trans_add)

// Special flag. It indicates a second texture has been found
// Its type depends ... either a normal textue or a normal map
#define AI_IRRMESH_EXTRA_2ND_TEXTURE        0x100000

// ---------------------------------------------------------------------------
/** Base class for the Irr and IrrMesh importers.
 *
 *  Declares some irrlight-related xml parsing utilities and provides tools
 *  to load materials from IRR and IRRMESH files.
 */
class IrrlichtBase
{
protected:

    /** @brief Data structure for a simple name-value property
     */
    template <class T>
    struct Property
    {
        std::string name;
        T value;
    };

    typedef Property<uint32_t>      HexProperty;
    typedef Property<std::string>   StringProperty;
    typedef Property<bool>          BoolProperty;
    typedef Property<float>         FloatProperty;
    typedef Property<aiVector3D>    VectorProperty;
    typedef Property<int>           IntProperty;

    /** XML reader instance
     */
  irr::io::IrrXMLReader* reader;

    // -------------------------------------------------------------------
    /** Parse a material description from the XML
     *  @return The created material
     *  @param matFlags Receives AI_IRRMESH_MAT_XX flags
     */
    aiMaterial* ParseMaterial(unsigned int& matFlags);

    // -------------------------------------------------------------------
    /** Read a property of the specified type from the current XML element.
     *  @param out Recives output data
     */
    void ReadHexProperty    (HexProperty&    out);
    void ReadStringProperty (StringProperty& out);
    void ReadBoolProperty   (BoolProperty&   out);
    void ReadFloatProperty  (FloatProperty&  out);
    void ReadVectorProperty (VectorProperty&  out);
    void ReadIntProperty    (IntProperty&    out);
};


// ------------------------------------------------------------------------------------------------
// Unpack a hex color, e.g. 0xdcdedfff
inline void ColorFromARGBPacked(uint32_t in, aiColor4D& clr)
{
    clr.a = ((in >> 24) & 0xff) / 255.f;
    clr.r = ((in >> 16) & 0xff) / 255.f;
    clr.g = ((in >>  8) & 0xff) / 255.f;
    clr.b = ((in      ) & 0xff) / 255.f;
}


} // end namespace Assimp

#endif // !! INCLUDED_AI_IRRSHARED_H
