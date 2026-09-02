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

#include "folder_manager.h"

#include <algorithm>
#include <fstream>

#include "gtest/gtest.h"

#include "util/json_writer.h"
#include "util/env_var_wrapper.h"

namespace fs {
Folder::Folder(std::filesystem::path root_path, std::string name) noexcept : folder((root_path / name).lexically_normal()) {
    clear();
    // Don't actually create the folder yet, as we will do it on demand
}
Folder::~Folder() noexcept { clear(); }
Folder::Folder(Folder&& other) noexcept : actually_created(other.actually_created), folder(other.folder) { other.folder.clear(); }
Folder& Folder::operator=(Folder&& other) noexcept {
    folder = other.folder;
    actually_created = other.actually_created;
    other.folder.clear();
    return *this;
}

void Folder::check_if_first_use() {
    if (!actually_created) {
        if (!::testing::internal::InDeathTestChild()) {
            std::error_code err;
            if (!std::filesystem::create_directories(folder, err)) {
                std::cerr << "Failed to create folder " << folder << " because " << err.message() << "\n";
            }
        }
        actually_created = true;
    }
}

std::filesystem::path Folder::write_manifest(std::filesystem::path const& name, std::string const& contents) {
    check_if_first_use();
    std::filesystem::path out_path = (folder / name).lexically_normal();
    if (!::testing::internal::InDeathTestChild()) {
        auto file = std::ofstream(out_path, std::ios_base::trunc | std::ios_base::out);
#if defined(_WIN32)
        // If the manifest path is too long for Windows, retry with the \\?\ absolute-path prefix.
        if (!file) {
            std::wstring long_path = L"\\\\?\\" + std::filesystem::absolute(out_path).native();
            file = std::ofstream(long_path.c_str(), std::ios_base::trunc | std::ios_base::out);
        }
#endif
        if (!file) {
            std::cerr << "Failed to create manifest " << name << " at " << out_path << "\n";
            return out_path;
        }
        file << contents << std::endl;
    }
    insert_file_to_tracking(name);
    return out_path;
}

// close file handle, delete file, remove `name` from managed file list.
void Folder::remove(std::filesystem::path const& name) {
    check_if_first_use();
    std::filesystem::path out_path = folder / name;
    if (!::testing::internal::InDeathTestChild()) {
        std::error_code err;
        if (!std::filesystem::remove(out_path, err)) {
            std::cerr << "Failed to remove file " << name << " at " << out_path << " because " << err.message() << "\n";
        }
    }

    auto found = std::find(added_files.begin(), added_files.end(), name);
    if (found != added_files.end()) {
        added_files.erase(found);
    } else {
        std::cout << "File " << name << " not in tracked files of folder " << folder << ".\n";
    }
}

// copy file into this folder
std::filesystem::path Folder::copy_file(std::filesystem::path const& file, std::filesystem::path const& new_name) {
    check_if_first_use();
    insert_file_to_tracking(new_name);

    auto new_filepath = folder / new_name;
    if (!::testing::internal::InDeathTestChild()) {
        std::error_code err;
        if (!std::filesystem::copy_file(file, new_filepath, err)) {
            std::cerr << "Failed to copy file " << file << " to " << new_filepath << " because " << err.message() << "\n";
        }
    }
    return new_filepath;
}

std::vector<std::filesystem::path> Folder::get_files() const { return added_files; }

std::filesystem::path Folder::add_symlink(std::filesystem::path const& target, std::filesystem::path const& link_name) {
    check_if_first_use();

    if (!::testing::internal::InDeathTestChild()) {
        std::error_code err;
        std::filesystem::create_symlink(target, folder / link_name, err);
        if (err.value() != 0) {
            std::cerr << "Failed to create symlink with target" << target << " with name " << folder / link_name << " because "
                      << err.message() << "\n";
        }
    }
    insert_file_to_tracking(link_name);
    return folder / link_name;
}
void Folder::insert_file_to_tracking(std::filesystem::path const& name) {
    auto found = std::find(added_files.begin(), added_files.end(), name);
    if (found != added_files.end()) {
        std::cout << "Overwriting manifest " << name << ". Was this intended?\n";
    } else {
        added_files.emplace_back(name);
    }
}

void Folder::clear() const noexcept {
    if (!::testing::internal::InDeathTestChild()) {
        std::error_code err;
        std::filesystem::remove_all(folder, err);
        if (err.value() != 0) {
            std::cerr << "Failed to remove folder " << folder << " because " << err.message() << "\n";
        }
    }
}

std::filesystem::path category_path_name(ManifestCategory category) {
    if (category == ManifestCategory::settings) return "settings.d";
    if (category == ManifestCategory::implicit_layer) return "implicit_layer.d";
    if (category == ManifestCategory::explicit_layer)
        return "explicit_layer.d";
    else
        return "icd.d";
}

FileSystemManager::FileSystemManager()
    : root_folder(std::filesystem::path(TEST_EXECUTION_DIRECTORY),
                  std::string(::testing::UnitTest::GetInstance()->GetInstance()->current_test_suite()->name()) + "_" +
                      ::testing::UnitTest::GetInstance()->current_test_info()->name()) {
    // Clean out test folder in case a previous run's files are still around
    root_folder.clear();

    auto const& root = root_folder.location();
    folders.try_emplace(ManifestLocation::null, root, std::string("null_dir"));
    folders.try_emplace(ManifestLocation::driver, root, std::string("icd_manifests"));
    folders.try_emplace(ManifestLocation::driver_env_var, root, std::string("icd_env_vars_manifests"));
    folders.try_emplace(ManifestLocation::explicit_layer, root, std::string("explicit_layer_manifests"));
    folders.try_emplace(ManifestLocation::explicit_layer_env_var, root, std::string("explicit_env_var_layer_folder"));
    folders.try_emplace(ManifestLocation::explicit_layer_add_env_var, root, std::string("explicit_add_env_var_layer_folder"));
    folders.try_emplace(ManifestLocation::implicit_layer, root, std::string("implicit_layer_manifests"));
    folders.try_emplace(ManifestLocation::implicit_layer_env_var, root, std::string("implicit_env_var_layer_manifests"));
    folders.try_emplace(ManifestLocation::implicit_layer_add_env_var, root, std::string("implicit_add_env_var_layer_manifests"));
    folders.try_emplace(ManifestLocation::override_layer, root, std::string("override_layer_manifests"));
#if defined(_WIN32)
    folders.try_emplace(ManifestLocation::windows_app_package, root, std::string("app_package_manifests"));
#endif
#if defined(__APPLE__)
    folders.try_emplace(ManifestLocation::macos_bundle, root, std::string("macos_bundle"));
#endif
    folders.try_emplace(ManifestLocation::settings_location, root, std::string("settings_location"));
    folders.try_emplace(ManifestLocation::unsecured_driver, root, std::string("unsecured_driver"));
    folders.try_emplace(ManifestLocation::unsecured_explicit_layer, root, std::string("unsecured_explicit_layer"));
    folders.try_emplace(ManifestLocation::unsecured_implicit_layer, root, std::string("unsecured_implicit_layer"));
    folders.try_emplace(ManifestLocation::unsecured_settings, root, std::string("unsecured_settings"));
}

fs::Folder& FileSystemManager::get_folder(ManifestLocation location) noexcept { return folders.at(location); }
fs::Folder const& FileSystemManager::get_folder(ManifestLocation location) const noexcept { return folders.at(location); }

Folder* FileSystemManager::get_folder_for_given_path(std::string_view given_path) noexcept {
    for (auto const& [manifest_location, folder] : folders) {
        if (folder.location() == given_path) {
            return &folders.at(manifest_location);
        }
    }
    for (auto const& [redirected_path, redirected_location] : redirected_paths) {
        if (redirected_path == given_path) {
            return &folders.at(redirected_location);
        }
        const auto mismatch_pair =
            std::mismatch(given_path.begin(), given_path.end(), redirected_path.begin(), redirected_path.end());
        if (mismatch_pair.second == redirected_path.end()) return &folders.at(redirected_location);
    }
    return nullptr;
}

bool FileSystemManager::is_folder_path(std::filesystem::path const& path) const noexcept {
    for (auto const& [location, folder] : folders) {
        if (path == folder.location()) {
            return true;
        }
    }
    return false;
}

void FileSystemManager::add_path_redirect(std::filesystem::path const& redirected_path, ManifestLocation location) {
    redirected_paths[redirected_path.string()] = location;
}

std::filesystem::path FileSystemManager::get_real_path_of_redirected_path(std::filesystem::path const& path) const {
    if (redirected_paths.count(path.string()) > 0) {
        return folders.at(redirected_paths.at(path.lexically_normal().string())).location();
    }
    for (auto const& [redirected_path_str, redirected_location] : redirected_paths) {
        std::filesystem::path redirected_path = redirected_path_str;
        const auto mismatch_pair = std::mismatch(path.begin(), path.end(), redirected_path.begin(), redirected_path.end());
        if (mismatch_pair.second == redirected_path.end()) return folders.at(redirected_location).location();
    }
    return std::filesystem::path{};
}

std::filesystem::path FileSystemManager::get_path_redirect_by_manifest_location(ManifestLocation location) const {
    for (auto const& [path, redirected_location] : redirected_paths) {
        if (redirected_location == location) {
            return path;
        }
    }
    return folders.at(ManifestLocation::null).location();
}

}  // namespace fs
