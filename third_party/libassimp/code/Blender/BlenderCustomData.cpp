#include "BlenderCustomData.h"
#include "BlenderDNA.h"
#include <array>
#include <functional>

namespace Assimp {
    namespace Blender {
        /**
        *   @brief  read/convert of Structure array to memory
        */
        template<typename T>
        bool read(const Structure &s, T *p, const size_t cnt, const FileDatabase &db) {
            for (size_t i = 0; i < cnt; ++i) {
                T read;
                s.Convert(read, db);
                *p = read;
                p++;
            }
            return true;
        }

        /**
        *   @brief  pointer to function read memory for n CustomData types
        */
        typedef bool        (*PRead)(ElemBase *pOut, const size_t cnt, const FileDatabase &db);
        typedef ElemBase *  (*PCreate)(const size_t cnt);
        typedef void(*PDestroy)(ElemBase *);

#define IMPL_STRUCT_READ(ty)                                                    \
        bool read##ty(ElemBase *v, const size_t cnt, const FileDatabase &db) {  \
        ty *ptr = dynamic_cast<ty*>(v);                                         \
        if (nullptr == ptr) {                                                   \
            return false;                                                       \
        }                                                                       \
        return read<ty>(db.dna[#ty], ptr, cnt, db);                             \
        }

#define IMPL_STRUCT_CREATE(ty)                                                  \
        ElemBase *create##ty(const size_t cnt) {                                \
            return new ty[cnt];                                                 \
        }

#define IMPL_STRUCT_DESTROY(ty)                                                 \
        void destroy##ty(ElemBase *pE) {                                        \
            ty *p = dynamic_cast<ty *>(pE);                                     \
            delete[]p;                                                          \
        }

        /**
        *   @brief  helper macro to define Structure functions
        */
#define IMPL_STRUCT(ty)                                                         \
        IMPL_STRUCT_READ(ty)                                                    \
        IMPL_STRUCT_CREATE(ty)                                                  \
        IMPL_STRUCT_DESTROY(ty)

        // supported structures for CustomData
        IMPL_STRUCT(MVert)
        IMPL_STRUCT(MEdge)
        IMPL_STRUCT(MFace)
        IMPL_STRUCT(MTFace)
        IMPL_STRUCT(MTexPoly)
        IMPL_STRUCT(MLoopUV)
        IMPL_STRUCT(MLoopCol)
        IMPL_STRUCT(MPoly)
        IMPL_STRUCT(MLoop)

        /**
        *   @brief  describes the size of data and the read function to be used for single CustomerData.type
        */
        struct CustomDataTypeDescription {
            PRead Read;                         ///< function to read one CustomData type element
            PCreate Create;                       ///< function to allocate n type elements
            PDestroy Destroy;

            CustomDataTypeDescription(PRead read, PCreate create, PDestroy destroy)
                : Read(read)
                , Create(create)
                , Destroy(destroy)
            {}
        };


        /**
        *   @brief  helper macro to define Structure type specific CustomDataTypeDescription
        *   @note   IMPL_STRUCT_READ for same ty must be used earlier to implement the typespecific read function
        */
#define DECL_STRUCT_CUSTOMDATATYPEDESCRIPTION(ty)           \
        CustomDataTypeDescription{&read##ty, &create##ty, &destroy##ty}

        /**
        *   @brief  helper macro to define CustomDataTypeDescription for UNSUPPORTED type
        */
#define DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION          \
        CustomDataTypeDescription{nullptr, nullptr, nullptr}

        /**
        *   @brief  descriptors for data pointed to from CustomDataLayer.data
        *   @note   some of the CustomData uses already well defined Structures
        *           other (like CD_ORCO, ...) uses arrays of rawtypes or even arrays of Structures
        *           use a special readfunction for that cases
        */
        std::array<CustomDataTypeDescription, CD_NUMTYPES> customDataTypeDescriptions = { {
            DECL_STRUCT_CUSTOMDATATYPEDESCRIPTION(MVert),
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_STRUCT_CUSTOMDATATYPEDESCRIPTION(MEdge),
            DECL_STRUCT_CUSTOMDATATYPEDESCRIPTION(MFace),
            DECL_STRUCT_CUSTOMDATATYPEDESCRIPTION(MTFace),
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,

            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_STRUCT_CUSTOMDATATYPEDESCRIPTION(MTexPoly),
            DECL_STRUCT_CUSTOMDATATYPEDESCRIPTION(MLoopUV),
            DECL_STRUCT_CUSTOMDATATYPEDESCRIPTION(MLoopCol),
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,

            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_STRUCT_CUSTOMDATATYPEDESCRIPTION(MPoly),
            DECL_STRUCT_CUSTOMDATATYPEDESCRIPTION(MLoop),
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,

            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,

            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION,
            DECL_UNSUPPORTED_CUSTOMDATATYPEDESCRIPTION
        }};


        bool isValidCustomDataType(const int cdtype) {
            return cdtype >= 0 && cdtype < CD_NUMTYPES;
        }

        bool readCustomData(std::shared_ptr<ElemBase> &out, const int cdtype, const size_t cnt, const FileDatabase &db) {
            if (!isValidCustomDataType(cdtype)) {
                throw Error((Formatter::format(), "CustomData.type ", cdtype, " out of index"));
            }

            const CustomDataTypeDescription cdtd = customDataTypeDescriptions[cdtype];
            if (cdtd.Read && cdtd.Create && cdtd.Destroy && cnt > 0) {
                // allocate cnt elements and parse them from file
                out.reset(cdtd.Create(cnt), cdtd.Destroy);
                return cdtd.Read(out.get(), cnt, db);
            }
            return false;
        }

        std::shared_ptr<CustomDataLayer> getCustomDataLayer(const CustomData &customdata, const CustomDataType cdtype, const std::string &name) {
            for (auto it = customdata.layers.begin(); it != customdata.layers.end(); ++it) {
                if (it->get()->type == cdtype && name == it->get()->name) {
                    return *it;
                }
            }
            return nullptr;
        }

        const ElemBase * getCustomDataLayerData(const CustomData &customdata, const CustomDataType cdtype, const std::string &name)
        {
            const std::shared_ptr<CustomDataLayer> pLayer = getCustomDataLayer(customdata, cdtype, name);
            if (pLayer && pLayer->data) {
                return pLayer->data.get();
            }
            return nullptr;
        }
    }
}
