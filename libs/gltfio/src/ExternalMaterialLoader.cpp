#include <gltfio/MaterialProvider.h>

#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>

#include <math/mat4.h>

#include <utils/CString.h>
#include <tsl/robin_map.h>

#if GLTFIO_LITE
#include "gltfresources_lite.h"
#else
#include "gltfresources.h"
#endif

using namespace filament;
using namespace filament::math;
using namespace gltfio;
using namespace utils;

namespace {

    class ExternalMaterialLoader : public MaterialProvider {
    public:
        ExternalMaterialLoader(
                filament::Engine* engine, ExternalSourceMaterialResolver materialResolver,
                ExternalSourceMaterialInstantiator materialInstantiator);
        ~ExternalMaterialLoader();

        MaterialSource getSource() const noexcept override { return EXTERNAL; }

        filament::MaterialInstance* createMaterialInstance(MaterialKey* config, UvMap* uvmap,
                                                           const char* label) override;

        size_t getMaterialsCount() const noexcept override;
        const filament::Material* const* getMaterials() const noexcept override;
        void destroyMaterials() override;

        filament::Engine* mEngine;
        ExternalSourceMaterialResolver mResolveMaterial;
        ExternalSourceMaterialInstantiator mMaterialInstantiator;
        std::vector<filament::Material*> mMaterials;
        tsl::robin_map<utils::CString, filament::Material*> mCache;
    };

    ExternalMaterialLoader::ExternalMaterialLoader(
            Engine* engine, ExternalSourceMaterialResolver externalSource,
            ExternalSourceMaterialInstantiator materialInstantiator)
            : mEngine(engine)
            , mResolveMaterial(externalSource)
            , mMaterialInstantiator(materialInstantiator) {}

    ExternalMaterialLoader::~ExternalMaterialLoader() {}

    size_t ExternalMaterialLoader::getMaterialsCount() const noexcept {
        return mMaterials.size();
    }

    const Material* const* ExternalMaterialLoader::getMaterials() const noexcept {
        return mMaterials.data();
    }

    void ExternalMaterialLoader::destroyMaterials() {
        for (auto& iter : mCache) {
            mEngine->destroy(iter.second);
        }
        mMaterials.clear();
        mCache.clear();
    }

    MaterialInstance* ExternalMaterialLoader::createMaterialInstance(MaterialKey* config, UvMap* uvmap, const char* label) {
        const auto name = CString(label);
        constrainMaterial(config, uvmap);
        auto iter = mCache.find(name);
        Material* material;
        if (iter == mCache.end()) {
            material = mResolveMaterial(mEngine, config, uvmap, label);
            if (!material) {
                return nullptr;
            }
            mCache.emplace(std::make_pair(name, material));
            mMaterials.push_back(material);
        } else {
            material = iter->second;
        }

        return mMaterialInstantiator(mEngine, material, config, uvmap);
    }

} // anonymous namespace

namespace gltfio {

    MaterialProvider* createExternalMaterialLoader(
            filament::Engine* engine, ExternalSourceMaterialResolver materialResolver,
            ExternalSourceMaterialInstantiator materialInstantiator) {
        return new ExternalMaterialLoader(engine, materialResolver, materialInstantiator);
    }

} // namespace gltfio
