#include "draco/io/file_writer_utils.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <string>

#include "draco/draco_features.h"

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

bool DirectoryExists(const std::string &path) {
  struct stat path_stat;

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

  const bool directory_exists = DirectoryExists(path);
  return directory_exists;
}

}  // namespace draco
