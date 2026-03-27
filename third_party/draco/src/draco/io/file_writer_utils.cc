#include "draco/io/file_writer_utils.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <string>

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "ghc/filesystem.hpp"
#endif  // DRACO_TRANSCODER_SUPPORTED

namespace draco {

void SplitPathPrivate(const std::string &full_path,
                      std::string *out_folder_path,
                      std::string *out_file_name) {
  const auto pos = full_path.find_last_of("/\\");
  if (pos != std::string::npos) {
    if (out_folder_path) {
      *out_folder_path = full_path.substr(0, pos);
    }
    if (out_file_name) {
      *out_file_name = full_path.substr(pos + 1, full_path.length());
    }
  } else {
    if (out_folder_path) {
      *out_folder_path = ".";
    }
    if (out_file_name) {
      *out_file_name = full_path;
    }
  }
}

bool DirectoryExists(const std::string &path_arg) {
  struct stat path_stat;
  std::string path = path_arg;

#if defined(_WIN32) && !defined(__MINGW32__)
  // Avoid a silly windows issue: stat() will fail on a drive letter missing the
  // trailing slash. To keep it simple, append a path separator to all paths.
  if (!path.empty() && path.back() != '\\' && path.back() != '/') {
    path.append("\\");
  }
#endif

  // Check if |path| exists.
  if (stat(path.c_str(), &path_stat) != 0) {
    return false;
  }

  // Check if |path| is a directory.
  if (path_stat.st_mode & S_IFDIR) {
    return true;
  }
  return false;
}

bool CheckAndCreatePathForFile(const std::string &filename) {
  std::string path;
  std::string basename;
  SplitPathPrivate(filename, &path, &basename);

#ifdef DRACO_TRANSCODER_SUPPORTED
  const ghc::filesystem::path ghc_path(path);
  ghc::filesystem::create_directories(ghc_path);
#endif  // DRACO_TRANSCODER_SUPPORTED
  return DirectoryExists(path);
}

}  // namespace draco
