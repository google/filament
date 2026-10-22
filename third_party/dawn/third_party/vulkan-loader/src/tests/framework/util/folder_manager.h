/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 */

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "test_defines.h"

namespace fs {

class Folder {
   public:
    explicit Folder(std::filesystem::path root_path, std::string name) noexcept;
    ~Folder() noexcept;
    Folder(Folder const&) = delete;
    Folder& operator=(Folder const&) = delete;
    Folder(Folder&& other) noexcept;
    Folder& operator=(Folder&& other) noexcept;

    // Add a manifest to the folder
    std::filesystem::path write_manifest(std::filesystem::path const& name, std::string const& contents);

    // close file handle, delete file, remove `name` from managed file list.
    void remove(std::filesystem::path const& name);

    // Remove all contents in the path
    void clear() const noexcept;

    // copy file into this folder with name `new_name`. Returns the full path of the file that was copied
    std::filesystem::path copy_file(std::filesystem::path const& file, std::filesystem::path const& new_name);

    // location of the managed folder
    std::filesystem::path const& location() const { return folder; }

    std::vector<std::filesystem::path> get_files() const;

    // Create a symlink in this folder to target with the filename set to link_name
    std::filesystem::path add_symlink(std::filesystem::path const& target, std::filesystem::path const& link_name);

   private:
    bool actually_created = false;
    std::filesystem::path folder;
    std::vector<std::filesystem::path> added_files;

    void insert_file_to_tracking(std::filesystem::path const& name);
    void check_if_first_use();
};

class FileSystemManager {
    fs::Folder root_folder;
    std::unordered_map<ManifestLocation, Folder> folders;

    // paths that this folder appears as to the loader (ex /usr/share/vulkan/explicit_layer.d is in the ManifestLocation::icd folder
    std::unordered_map<std::string, ManifestLocation> redirected_paths;

   public:
    FileSystemManager();

    Folder& get_folder(ManifestLocation location) noexcept;
    Folder const& get_folder(ManifestLocation location) const noexcept;

    // Gets a pointer to the folder that given_path points to. This includes redirected paths as well as the exact path of folders
    Folder* get_folder_for_given_path(std::string_view given_path) noexcept;

    bool is_folder_path(std::filesystem::path const& path) const noexcept;

    void add_path_redirect(std::filesystem::path const& apparent_path, ManifestLocation location);

    // Returns the first redirected path pointing at by a manifest location. Returns the NULL location if one isn't found
    std::filesystem::path get_path_redirect_by_manifest_location(ManifestLocation location) const;

    // Returns the real path that a redirected path points to. Returns an empty path if no redirect is found
    std::filesystem::path get_real_path_of_redirected_path(std::filesystem::path const& redirected_path) const;
};

}  // namespace fs
