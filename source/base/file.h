//
// Created by zerdzhong on 2020/1/30.
//

#ifndef FFNETWORK_FILE_H
#define FFNETWORK_FILE_H

#include <vector>
#include <string>
#include "unique_fd.h"

namespace ffbase {

class Mapping;

enum class FilePermission {
  kRead,
  kWrite,
  kReadWrite,
};

ffbase::UniqueFD OpenFile(const char *path, bool need_create,
                          FilePermission permission);

ffbase::UniqueFD OpenFile(const ffbase::UniqueFD& base_dir, const char* path,
                          bool need_create, FilePermission permission);

ffbase::UniqueFD OpenDirectory(const char *path, bool need_create,
                               FilePermission permission);

ffbase::UniqueFD OpenDirectory(const ffbase::UniqueFD &base_dir,
                               const char *path, bool need_create,
                               FilePermission permission);

bool IsDirectory(const ffbase::UniqueFD& base_dir);
bool IsDirectory(const ffbase::UniqueFD& base_dir, const char* path);
bool IsFile(const char* path);

bool TruncateFile(const ffbase::UniqueFD& file, size_t size);

bool FileExists(const ffbase::UniqueFD& base_dir, const char* path);

bool UnlinkDirectory(const char* path);
bool UnlinkDirectory(const ffbase::UniqueFD& base_dir, const char* path);

bool UnlinkFile(const char* path);
bool UnlinkFile(const ffbase::UniqueFD& base_dir, const char* path);


ffbase::UniqueFD CreateDirectory(const ffbase::UniqueFD &base_dir,
                                 const std::vector<std::string>& components,
                                 FilePermission permission);

bool WriteAtomically(const ffbase::UniqueFD &base_dir, const char *file_name,
                     const Mapping& data);

std::string CreateTemporaryDirectory();

class ScopedTemporaryDirectory {
public:
  ScopedTemporaryDirectory();

  ~ScopedTemporaryDirectory();

  const std::string& path() const { return path_; }
  const UniqueFD& fd() { return dir_fd_; }

private:
  std::string path_;
  UniqueFD dir_fd_;
};

}// namespace ffbase

#endif // FFNETWORK_FILE_H
