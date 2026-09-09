// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/dawn/common/DRMUtils.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include <algorithm>
#include <cctype>
#include <memory>
#include <string_view>

namespace dawn {
namespace {

struct DirectoryCloser {
    void operator()(DIR* directory) const { closedir(directory); }
};

}  // namespace

bool IsDRMRenderNodeName(std::string_view nodeName) {
    constexpr std::string_view kRenderNodePrefix = "renderD";
    return nodeName.starts_with(kRenderNodePrefix) && nodeName.size() > kRenderNodePrefix.size() &&
           std::ranges::all_of(nodeName.substr(kRenderNodePrefix.size()),
                               [](unsigned char character) { return std::isdigit(character); });
}

bool IsMatchingDRMRenderNode(std::string_view nodeName,
                             bool isCharacterDevice,
                             uint64_t nodeMajor,
                             uint64_t nodeMinor,
                             uint64_t renderMajor,
                             uint64_t renderMinor) {
    // Reject non-character files with render-node-like names. DRM render nodes are character
    // devices.
    return IsDRMRenderNodeName(nodeName) && isCharacterDevice && nodeMajor == renderMajor &&
           nodeMinor == renderMinor;
}

SystemHandle OpenDRMRenderNode(uint64_t renderMajor, uint64_t renderMinor) {
    constexpr char kDRMDirectory[] = "/dev/dri";
    std::unique_ptr<DIR, DirectoryCloser> drmDirectory(opendir(kDRMDirectory));
    if (drmDirectory == nullptr) {
        return {};
    }

    const int drmDirectoryFd = dirfd(drmDirectory.get());
    if (drmDirectoryFd < 0) {
        return {};
    }

    while (dirent* entry = readdir(drmDirectory.get())) {
        std::string_view entryName(entry->d_name);
        if (!IsDRMRenderNodeName(entryName)) {
            continue;
        }

        // Check st_rdev before opening to avoid waking unrelated GPUs. major() and minor()
        // extract the DRM device number to compare with the values reported by Vulkan. Recheck
        // the opened descriptor to guard against replacement between fstatat() and openat().
        struct stat nodeStat = {};
        if (fstatat(drmDirectoryFd, entry->d_name, &nodeStat, AT_SYMLINK_NOFOLLOW) != 0 ||
            !IsMatchingDRMRenderNode(entryName, S_ISCHR(nodeStat.st_mode), major(nodeStat.st_rdev),
                                     minor(nodeStat.st_rdev), renderMajor, renderMinor)) {
            continue;
        }

        SystemHandle renderNode = SystemHandle::Acquire(
            openat(drmDirectoryFd, entry->d_name, O_RDWR | O_CLOEXEC | O_NOFOLLOW));
        if (!renderNode.IsValid()) {
            continue;
        }

        nodeStat = {};
        if (fstat(renderNode.Get(), &nodeStat) == 0 &&
            IsMatchingDRMRenderNode(entryName, S_ISCHR(nodeStat.st_mode), major(nodeStat.st_rdev),
                                    minor(nodeStat.st_rdev), renderMajor, renderMinor)) {
            return renderNode;
        }
    }
    return {};
}

}  // namespace dawn
