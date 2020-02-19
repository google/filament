#pragma once

#include "BlenderDNA.h"
#include "BlenderScene.h"
#include <memory>

namespace Assimp {
    namespace Blender {
        /* CustomData.type from Blender (2.79b) */
        enum CustomDataType {
            CD_AUTO_FROM_NAME = -1,
            CD_MVERT = 0,
#ifdef DNA_DEPRECATED
            CD_MSTICKY = 1,  /* DEPRECATED */
#endif
            CD_MDEFORMVERT = 2,
            CD_MEDGE = 3,
            CD_MFACE = 4,
            CD_MTFACE = 5,
            CD_MCOL = 6,
            CD_ORIGINDEX = 7,
            CD_NORMAL = 8,
            /*	CD_POLYINDEX        = 9, */
            CD_PROP_FLT = 10,
            CD_PROP_INT = 11,
            CD_PROP_STR = 12,
            CD_ORIGSPACE = 13,  /* for modifier stack face location mapping */
            CD_ORCO = 14,
            CD_MTEXPOLY = 15,
            CD_MLOOPUV = 16,
            CD_MLOOPCOL = 17,
            CD_TANGENT = 18,
            CD_MDISPS = 19,
            CD_PREVIEW_MCOL = 20,  /* for displaying weightpaint colors */
            /*	CD_ID_MCOL          = 21, */
            CD_TEXTURE_MLOOPCOL = 22,
            CD_CLOTH_ORCO = 23,
            CD_RECAST = 24,

            /* BMESH ONLY START */
            CD_MPOLY = 25,
            CD_MLOOP = 26,
            CD_SHAPE_KEYINDEX = 27,
            CD_SHAPEKEY = 28,
            CD_BWEIGHT = 29,
            CD_CREASE = 30,
            CD_ORIGSPACE_MLOOP = 31,
            CD_PREVIEW_MLOOPCOL = 32,
            CD_BM_ELEM_PYPTR = 33,
            /* BMESH ONLY END */

            CD_PAINT_MASK = 34,
            CD_GRID_PAINT_MASK = 35,
            CD_MVERT_SKIN = 36,
            CD_FREESTYLE_EDGE = 37,
            CD_FREESTYLE_FACE = 38,
            CD_MLOOPTANGENT = 39,
            CD_TESSLOOPNORMAL = 40,
            CD_CUSTOMLOOPNORMAL = 41,

            CD_NUMTYPES = 42
        };

        /**
        *   @brief  check if given cdtype is valid (ie >= 0 and < CD_NUMTYPES)
        *   @param[in]  cdtype to check
        *   @return true when valid
        */
        bool isValidCustomDataType(const int cdtype);

        /**
        *   @brief  returns CustomDataLayer ptr for given cdtype and name
        *   @param[in]  customdata CustomData to search for wanted layer
        *   @param[in]  cdtype to search for
        *   @param[in]  name to search for
        *   @return CustomDataLayer * or nullptr if not found
        */
        std::shared_ptr<CustomDataLayer> getCustomDataLayer(const CustomData &customdata, CustomDataType cdtype, const std::string &name);

        /**
        *   @brief  returns CustomDataLayer data ptr for given cdtype and name
        *   @param[in]  customdata CustomData to search for wanted layer
        *   @param[in]  cdtype to search for
        *   @param[in]  name to search for
        *   @return * to struct data or nullptr if not found
        */
        const ElemBase * getCustomDataLayerData(const CustomData &customdata, CustomDataType cdtype, const std::string &name);
    }
}
