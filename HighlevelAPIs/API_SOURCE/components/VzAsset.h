#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzAsset : VzBaseComp
    {
        VzAsset(const VID vid, const std::string& originFrom, const std::string& typeName)
            : VzBaseComp(vid, originFrom, typeName) {}
        std::vector<VID> GetGLTFRoots();
        std::vector<VID> GetSkeletons();

        // Animator //
        __dojostruct Animator{
        public:
            enum class PlayMode { INIT_POSE, PLAY, PAUSE, };
        private:
            VID vidAsset_ = INVALID_VID;
            std::set<size_t> activatedAnimations_ = {}; // if this becomes GetAnimationCount(), apply all.
            size_t animationIndex_ = 0;
            int crossFadeAnimationIndex_ = -1;
            int crossFadePrevAnimationIndex_ = -1;
            double crossFadeDurationSec_ = 1.0;
            TimeStamp timer_ = {};
            double elapsedTimeSec_ = 0.0;
            double prevElapsedTimeSec_ = 0.0;
            double fixedUpdateTime_ = 1. / 60.; // default is 60 fps
            std::set<VID> associatedScenes_;
            PlayMode playMode_ = PlayMode::INIT_POSE;
            bool resetAnimation_ = true;
        public:
            Animator(VID vidAsset) { vidAsset_ = vidAsset; }

            size_t AddPlayScene(const VID vidScene) { associatedScenes_.insert(vidScene); return associatedScenes_.size(); }
            size_t RemovePlayScene(const VID vidScene) { associatedScenes_.erase(vidScene); return associatedScenes_.size(); }
            bool IsPlayScene(const VID vidScene) { return associatedScenes_.contains(vidScene); }
            VID GetAssetVID() { return vidAsset_; }
            size_t GetAnimationCount();
            std::string GetAnimationLabel(const int index);
            std::vector<std::string> GetAnimationLabels();
            std::string ActivateAnimation(const size_t index) { activatedAnimations_.insert(index); return GetAnimationLabel(index); }
            int ActivateAnimationByLabel(const std::string& label);
            void SetCrossFadeDuration(const int crossFadeAnimationIndex, const int crossFadePrevAnimationIndex, const double timeSec = 1.) { 
                crossFadeAnimationIndex_ = crossFadeAnimationIndex; crossFadePrevAnimationIndex_ = crossFadePrevAnimationIndex;  crossFadeDurationSec_ = timeSec;
            }
            float GetAnimationPlayTime(const size_t index);
            float GetAnimationPlayTimeByLabel(const std::string& label);

            void MovePlayTime(const double elsapsedTimeSec) { elapsedTimeSec_ = elsapsedTimeSec; }
            double GetPlayTime() { return elapsedTimeSec_; }
            void SetPlayMode(const PlayMode playMode) { playMode_ = playMode; resetAnimation_ = playMode == PlayMode::INIT_POSE; }
            PlayMode GetPlayMode() { return playMode_; }
            void Reset() { resetAnimation_ = true; }

            // note: this is called in the renderer (whose target is the associated scene) by default 
            void UpdateAnimation();
        };
        Animator* GetAnimator();              // this activates the camera manipulator
    };
}
