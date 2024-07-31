#include "VzAsset.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
#define COMP_ASSET_ANI(COMP, FAILRET)  VzAssetRes* COMP = gEngineApp.GetAssetRes(vidAsset_); assert(COMP->asset->getAssetInstanceCount() == 1); // later... for multi-instance cases
#define COMP_ASSET_ANI_INST(COMP, INST, FAILRET)  COMP_ASSET_ANI(COMP, FAILRET); FilamentInstance* INST = COMP->asset->getInstance(); if (INST == nullptr) return FAILRET; 
#define COMP_ASSET_ANI_INST_FANI(COMP, INST, FANI, FAILRET)  COMP_ASSET_ANI_INST(COMP, INST, FAILRET); filament::gltfio::Animator* FANI = INST->getAnimator(); if (FANI == nullptr) return FAILRET; 
    std::vector<VID> VzAsset::GetGLTFRoots()
    {
        COMP_ASSET(asset_res, std::vector<VID>());
        std::vector<VID> root_vids;
        std::copy(asset_res->rootVIDs.begin(), asset_res->rootVIDs.end(), std::back_inserter(root_vids));
        return root_vids;
    }

    std::vector<VID> VzAsset::GetSkeletons()
    {
        COMP_ASSET(asset_res, std::vector<VID>());
        return asset_res->skeletons;
    }
    size_t VzAsset::GetVariantsCount()
    {
        COMP_ASSET(asset_res, );
        FilamentInstance* finst = asset_res->asset->getInstance();
        return finst->getMaterialVariantCount();
    }
    std::string VzAsset::GetVariantName(int variantIndex)
    {
        std::string name = "";
        COMP_ASSET(asset_res, name);
        FilamentInstance* finst = asset_res->asset->getInstance();
        name = finst->getMaterialVariantName(variantIndex);
        return name;
    }
    void VzAsset::ApplyMaterialvariant(int variantIndex)
    {
        COMP_ASSET(asset_res, );
        FilamentInstance* finst = asset_res->asset->getInstance();
        finst->applyMaterialVariant(variantIndex);
    }

    VzAsset::Animator* VzAsset::GetAnimator()
    {
        COMP_ASSET(asset_res, nullptr);
        return &asset_res->animator;
    }

    size_t VzAsset::Animator::GetAnimationCount()
    {
        COMP_ASSET_ANI_INST_FANI(asset_res, finst, fani, 0);
        return fani->getAnimationCount();
    }
    std::string VzAsset::Animator::GetAnimationLabel(const int index)
    {
        COMP_ASSET_ANI_INST_FANI(asset_res, finst, fani, "");
        if ((size_t)index >= fani->getAnimationCount()) return "";
        return fani->getAnimationName((size_t)index);
    }
    std::vector<std::string> VzAsset::Animator::GetAnimationLabels()
    {
        COMP_ASSET_ANI_INST_FANI(asset_res, finst, fani, { {""} });
        size_t num_ani = fani->getAnimationCount();
        std::vector<std::string> labels;
        for (size_t i = 0; i < num_ani; ++i)
        {
            labels.push_back(fani->getAnimationName(i));
        }
        return labels;
    }
    int VzAsset::Animator::ActivateAnimationByLabel(const std::string& label)
    {
        COMP_ASSET_ANI_INST_FANI(asset_res, finst, fani, -1);
        size_t num_ani = fani->getAnimationCount();
        std::vector<std::string> labels;
        for (size_t i = 0; i < num_ani; ++i)
        {
            if (label == fani->getAnimationName(i))
            {
                ActivateAnimation(i);
                return i;
            }
        }
        return -1;
    }
    int VzAsset::Animator::DeactivateAnimationByLabel(const std::string& label)
    {
        COMP_ASSET_ANI_INST_FANI(asset_res, finst, fani, -1);
        size_t num_ani = fani->getAnimationCount();
        std::vector<std::string> labels;
        for (size_t i = 0; i < num_ani; ++i)
        {
            if (label == fani->getAnimationName(i))
            {
                activatedAnimations_.erase(i);
                return i;
            }
        }
        return -1;
    }
    int VzAsset::Animator::DeactivateAll()
    {
        COMP_ASSET_ANI_INST_FANI(asset_res, finst, fani, -1);
        size_t num_ani = fani->getAnimationCount();
        activatedAnimations_.clear();
        return num_ani;
    }
    float VzAsset::Animator::GetAnimationPlayTime(const size_t index)
    {
        COMP_ASSET_ANI_INST_FANI(asset_res, finst, fani, 0.f);
        const size_t animation_count = fani->getAnimationCount();
        if (index > animation_count) return 0.f;
        float duration = 0.f;
        if (animationIndex_ == animation_count) {
            for (size_t i = 0; i < animation_count; i++) 
            {
                duration = std::max(duration, fani->getAnimationDuration(i));
            }
        }
        else
        {
            duration = fani->getAnimationDuration(index);
        }
        return duration;
    }
    float VzAsset::Animator::GetAnimationPlayTimeByLabel(const std::string& label)
    {
        COMP_ASSET_ANI_INST_FANI(asset_res, finst, fani, 0.f);
        size_t num_ani = fani->getAnimationCount();
        std::vector<std::string> labels;
        for (size_t i = 0; i < num_ani; ++i)
        {
            if (label == fani->getAnimationName(i))
            {
                return GetAnimationPlayTime(i);
            }
        }
        return 0.f;
    }

    void VzAsset::Animator::ApplyAnimationTimeAt(const size_t index, const float elapsedTime) 
    {
        COMP_ASSET_ANI(asset_res, );
        FilamentInstance* finst = asset_res->asset->getInstance();
        if (finst == nullptr) return;
        filament::gltfio::Animator* fani = finst->getAnimator();
        if (fani == nullptr) return;

        const size_t animation_count = fani->getAnimationCount();
        if (animation_count <= index)
        {
            return;
        }
        fani->applyAnimation(index, elapsedTime);
    }

    void VzAsset::Animator::UpdateBoneMatrices()
    {
        COMP_ASSET_ANI(asset_res, );
        FilamentInstance* finst = asset_res->asset->getInstance();
        if (finst == nullptr) return;
        filament::gltfio::Animator* fani = finst->getAnimator();
        if (fani == nullptr) return;
        fani->updateBoneMatrices();
    }

    void VzAsset::Animator::UpdateAnimation()
    {
        //COMP_ASSET_ANI_INST_FANI(asset_res, vzGltfIO.assetResMaps, finst, fani, );
        COMP_ASSET_ANI(asset_res, );
        FilamentInstance* finst = asset_res->asset->getInstance();
        if (finst == nullptr) return;
        filament::gltfio::Animator* fani = finst->getAnimator();
        if (fani == nullptr) return;

        switch (playMode_)
        {
        case PlayMode::INIT_POSE:
            fani->resetBoneMatrices();
            resetAnimation_ = true;
            return;
        case PlayMode::PAUSE:
            timer_ = std::chrono::high_resolution_clock::now();
            return;
        case PlayMode::PLAY:
        default: break;
        }

        if (resetAnimation_) {
            timer_ = std::chrono::high_resolution_clock::now();
            prevElapsedTimeSec_ = elapsedTimeSec_;
            elapsedTimeSec_ = 0;
            resetAnimation_ = false;
            crossFadeAnimationIndex_ = crossFadePrevAnimationIndex_ = -1;
        }

        auto timestamp = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(timestamp - timer_);
        double delta_time = time_span.count(); // in sec.

        if (delta_time < fixedUpdateTime_)
        {
            //return; // WHY??? discontinuous?!
        }
        timer_ = timestamp;
        elapsedTimeSec_ += delta_time;

        const size_t animation_count = fani->getAnimationCount();
        for (size_t i = 0; i < animation_count; ++i)
        {
            if (activatedAnimations_.contains(i)) {
                fani->applyAnimation(i, elapsedTimeSec_);
            }
        }

        if (elapsedTimeSec_ < crossFadeDurationSec_) 
        {
            if (crossFadeAnimationIndex_ >= 0 && crossFadePrevAnimationIndex_ >= 0 && crossFadeAnimationIndex_ != crossFadePrevAnimationIndex_) 
            {
                const double previousSeconds = prevElapsedTimeSec_ + delta_time;
                const float lerpFactor = elapsedTimeSec_ / crossFadeDurationSec_;
                fani->applyAnimation(crossFadeAnimationIndex_, elapsedTimeSec_);
                fani->applyCrossFade(crossFadePrevAnimationIndex_, previousSeconds, lerpFactor);
            }
        }
        else
        {
            crossFadeAnimationIndex_ = crossFadePrevAnimationIndex_ = -1;
        }
        fani->updateBoneMatrices();
    }
}
