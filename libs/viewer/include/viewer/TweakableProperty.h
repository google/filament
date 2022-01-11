/////////////////////////////////////////////////////////////////
// Utility functions for our material tweaking extensions
// Shapr3D
/////////////////////////////////////////////////////////////////

#pragma once

#include <imgui.h>
#include <filagui/ImGuiExtensions.h>
#include <viewer/CustomFileDialogs.h>
//#include <filagui/imfilebrowser.h>

#include <utils/Path.h>

#include <math/mat4.h>
#include <math/vec3.h>

#include <vector>
#include <functional>
#include <string>
#include <type_traits>
#include <iostream>

// This is really nasty, instantiated in SimpleViewer.cpp
extern std::string g_ArtRootPathStr;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Common constants for tweakable properties
//////////////////////////////////////////////////////////////////////////////////////////////////
constexpr int MAX_FILENAME_LENGTH = 1024;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Templated alias to have a single validator for the types that we allow to be tweaked (float, filamenth::math floatNs, etc.)
//////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, typename = std::enable_if_t< std::is_same_v<float, T> || std::is_same_v<filament::math::float2, T> || std::is_same_v<filament::math::float3, T> || std::is_same_v<filament::math::float4, T> > >
using IsValidTweakableType = T;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Get how many scalars the given tweak-enabled type has
//////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, typename = IsValidTweakableType<T> > constexpr int dimCount() {
    return T::SIZE; 
}

template<> constexpr int dimCount<float>() { 
    return 1; 
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// ImGui widgets require pointers to their outputs and the following function creates these
//////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, typename = IsValidTweakableType<T> >
constexpr float* getPointerTo(T& instance) { 
    return instance.v; 
}

template <> constexpr float* getPointerTo<float>(float& instance) { 
    return &instance; 
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// A tweakable property with its own ImGui-based UI
//////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, bool MayContainFile = false, bool IsColor = true, bool IsDerivable = false, typename = IsValidTweakableType<T> >
struct TweakableProperty {
    TweakableProperty() {}
    TweakableProperty(const T& prop) : value(prop) {}

    void addWidget(const char* label, float min = 0.0f, float max = 1.0f, const char* format = "%.3f", float power = 1.0f) {
        ImGui::LabelText(label, "%s", label);
        if constexpr (IsDerivable) {
            std::string customDerivableLabel = std::string("Use derived value for: ") + label;
            ImGui::Checkbox(customDerivableLabel.c_str(), &useDerivedQuantity);
        }
        if constexpr (MayContainFile) {
            std::string customTextureLabel = std::string("Use texture: ") + label;
            ImGui::Checkbox(customTextureLabel.c_str(), &isFile);
            if (!isFile) {
                drawValueWidget(label, min, max, format, power);
            }
            else {
                static char fileDialogResult[MAX_FILENAME_LENGTH];
                ImGui::LabelText("Filename", "%s", filename.asString().c_str());
                std::string loadTextureLabel = std::string("Load##") + label;
                if (ImGui::Button(loadTextureLabel.c_str()) && SD_OpenFileDialog(fileDialogResult)) {
                    utils::Path fileDialogPath(fileDialogResult);
                    filename = fileDialogPath.makeRelativeTo(g_ArtRootPathStr);
                }

                if (!filename.empty()) {
                    ImGui::SameLine();
                    if (ImGui::Button("Reload")) {
                        doRequestReload = true;
                    }
                }
            }
        }
        else {
            drawValueWidget(label, min, max, format, power);
        }
        ImGui::Separator();
        ImGui::Spacing();
    }

    void drawValueWidget(const char* label, float min = 0.0f, float max = 1.0f, const char* format = "%.3f", float power = 1.0f) {
        if (useDerivedQuantity) return;
        if constexpr (dimCount<T>() == 4) {
            if constexpr (IsColor) {
                ImGui::ColorEdit4(label, getPointerTo(value));
            } else {
                ImGui::SliderFloat4(label, getPointerTo(value), min, max, format, power);
            }
        }
        else if constexpr (dimCount<T>() == 3) {
            if constexpr (IsColor) {
                ImGui::ColorEdit3(label, getPointerTo(value));
            }
            else {
                ImGui::SliderFloat3(label, getPointerTo(value), min, max, format, power);
            }
        }
        else if constexpr (dimCount<T>() == 2) {
            ImGui::SliderFloat2(label, getPointerTo(value), min, max, format, power);
        }
        else {
            ImGui::SliderFloat(label, getPointerTo(value), min, max, format, power);
        }
    }

    float* asPointer() { return getPointerTo(value); }

    T value{};
    bool isFile{};
    bool useDerivedQuantity{};
    bool doRequestReload{};

//private:
    class FilenameHolder {
        std::string mFilename{};

    public:
        FilenameHolder() : mFilename() {}
        FilenameHolder(const FilenameHolder&) = delete;
        FilenameHolder(const FilenameHolder&& o) {
            mFilename = std::move(o.mFilename);
        }

        FilenameHolder& operator=(const FilenameHolder&) = delete;
        FilenameHolder& operator=(const FilenameHolder&&) = delete;

        FilenameHolder& operator=(const std::string& filename) {
            utils::Path path(filename);
            if (path.isAbsolute() && !g_ArtRootPathStr.empty()) {
                std::cout << "Making path '" << filename << "' relative to '" << g_ArtRootPathStr << "'" << std::endl;
                mFilename = path.makeRelativeTo(g_ArtRootPathStr).c_str();
            } else {
                mFilename = filename;
            }
            return *this;
        }

        operator std::string() {
            utils::Path path(mFilename);
            if (path.isAbsolute() && !g_ArtRootPathStr.empty()) {
                std::cout << "Warning! Stored path is absolute, but art root is set!" << std::endl;
                std::cout << "\tMaking path '" << mFilename << "' relative to '" << g_ArtRootPathStr << "'" << std::endl;
                mFilename = path.makeRelativeTo(g_ArtRootPathStr).c_str();
            }
            return mFilename;
        }

        const std::string asString() const {
            return mFilename;
        }

        bool empty() { return mFilename.empty(); }
        void clear() { mFilename.clear(); }
    };
    
    //std::string filename{};
    FilenameHolder filename{};
};

template <typename T, bool IsColor = true, typename = IsValidTweakableType<T> >
using TweakablePropertyTextured = TweakableProperty<T, true, IsColor>;

template <typename T, bool IsColor = true, typename = IsValidTweakableType<T> >
using TweakablePropertyDerivableTextured = TweakableProperty<T, true, IsColor, true>;

template <typename T, bool IsColor = true, typename = IsValidTweakableType<T> >
using TweakablePropertyDerivable = TweakableProperty<T, false, IsColor, true>;
