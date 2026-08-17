#pragma once
#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>
#include "../../common/SampleConfig.h"
#include <utils/CString.h>
#include <memory>
#include <utils/FixedCapacityVector.h>

utils::FixedCapacityVector<utils::CString> getSampleNames();

std::unique_ptr<FilamentApp2> dispatchSample(const utils::CString& name, SampleConfig config, filament::app::DisplayManager* dm, filament::app::AssetLoader* loader);
