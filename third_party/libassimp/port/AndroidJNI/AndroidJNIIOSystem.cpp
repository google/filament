/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2016, assimp team

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

/** @file Android extension of DefaultIOSystem using the standard C file functions */


#include <assimp/config.h>
#include <android/api-level.h>
#if __ANDROID__ and __ANDROID_API__ > 9 and defined(AI_CONFIG_ANDROID_JNI_ASSIMP_MANAGER_SUPPORT)

#include <libgen.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_activity.h>
#include <assimp/ai_assert.h>
#include <assimp/port/AndroidJNI/AndroidJNIIOSystem.h>
#include <assimp/DefaultIOStream.h>
#include <fstream>

using namespace Assimp;

// ------------------------------------------------------------------------------------------------
// Constructor.
AndroidJNIIOSystem::AndroidJNIIOSystem(ANativeActivity* activity)
{
	AndroidActivityInit(activity);
}

// ------------------------------------------------------------------------------------------------
// Destructor.
AndroidJNIIOSystem::~AndroidJNIIOSystem()
{
	// nothing to do here
}

// ------------------------------------------------------------------------------------------------
// Tests for the existence of a file at the given path.
bool AndroidJNIIOSystem::Exists( const char* pFile) const
{
	AAsset* asset = AAssetManager_open(mApkAssetManager, pFile,
			AASSET_MODE_UNKNOWN);
	FILE* file = ::fopen( (mApkWorkspacePath + getOsSeparator() + std::string(pFile)).c_str(), "rb");

	if (!asset && !file)
		{
		__android_log_print(ANDROID_LOG_ERROR, "Assimp", "Asset manager can not find: %s", pFile);
		return false;
		}

	__android_log_print(ANDROID_LOG_ERROR, "Assimp", "Asset exists");
	if (file)
		::fclose( file);
	return true;
}

// ------------------------------------------------------------------------------------------------
// Inits Android extractor
void AndroidJNIIOSystem::AndroidActivityInit(ANativeActivity* activity)
{
	mApkWorkspacePath = activity->internalDataPath;
	mApkAssetManager = activity->assetManager;
}

// ------------------------------------------------------------------------------------------------
// Create the directory for the extracted resource
static int mkpath(std::string path, mode_t mode)
{
    if (mkdir(path.c_str(), mode) == -1) {
        switch(errno) {
            case ENOENT:
                if (mkpath(path.substr(0, path.find_last_of('/')), mode) == -1)
                    return -1;
                else
                    return mkdir(path.c_str(), mode);
            case EEXIST:
                return 0;
            default:
                return -1;
        }
    }

    return 0;
}

// ------------------------------------------------------------------------------------------------
// Extracts android asset
bool AndroidJNIIOSystem::AndroidExtractAsset(std::string name)
{
	std::string newPath = mApkWorkspacePath + getOsSeparator() + name;

	DefaultIOSystem io;

	// Do not extract if extracted already
	if ( io.Exists(newPath.c_str()) ) {
		__android_log_print(ANDROID_LOG_DEFAULT, "Assimp", "Asset already extracted");
		return true;
	}
	// Open file
	AAsset* asset = AAssetManager_open(mApkAssetManager, name.c_str(),
			AASSET_MODE_UNKNOWN);
	std::vector<char> assetContent;

	if (asset != NULL) {
		// Find size
		off_t assetSize = AAsset_getLength(asset);

		// Prepare input buffer
		assetContent.resize(assetSize);

		// Store input buffer
		AAsset_read(asset, &assetContent[0], assetSize);

		// Close
		AAsset_close(asset);

		// Prepare directory for output buffer
		std::string directoryNewPath = newPath;
		directoryNewPath = dirname(&directoryNewPath[0]);

		if (mkpath(directoryNewPath, S_IRUSR | S_IWUSR | S_IXUSR) == -1) {
			__android_log_print(ANDROID_LOG_ERROR, "assimp",
					"Can not create the directory for the output file");
		}

		// Prepare output buffer
		std::ofstream assetExtracted(newPath.c_str(),
				std::ios::out | std::ios::binary);
		if (!assetExtracted) {
			__android_log_print(ANDROID_LOG_ERROR, "assimp",
					"Can not open output file");
		}

		// Write output buffer into a file
		assetExtracted.write(&assetContent[0], assetContent.size());
		assetExtracted.close();

		__android_log_print(ANDROID_LOG_DEFAULT, "Assimp", "Asset extracted");
	} else {
		__android_log_print(ANDROID_LOG_ERROR, "assimp", "Asset not found: %s", name.c_str());
		return false;
	}
	return true;
}

// ------------------------------------------------------------------------------------------------
// Open a new file with a given path.
IOStream* AndroidJNIIOSystem::Open( const char* strFile, const char* strMode)
{
	ai_assert(NULL != strFile);
	ai_assert(NULL != strMode);

	std::string fullPath(mApkWorkspacePath + getOsSeparator() + std::string(strFile));
	if	(Exists(strFile))
		AndroidExtractAsset(std::string(strFile));

	FILE* file = ::fopen( fullPath.c_str(), strMode);

	if( NULL == file)
		return NULL;

	__android_log_print(ANDROID_LOG_ERROR, "assimp", "AndroidIOSystem: file %s opened", fullPath.c_str());
	return new DefaultIOStream(file, fullPath);
}

#undef PATHLIMIT
#endif // __ANDROID__ and __ANDROID_API__ > 9 and defined(AI_CONFIG_ANDROID_JNI_ASSIMP_MANAGER_SUPPORT)
