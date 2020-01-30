//
// Created by zerdzhong on 2020/1/30.
//

#include "file.h"
#include "mapping.h"
#include <fcntl.h>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>

namespace ffbase {

std::string CreateTemporaryDirectory() {
  char directory_name[] = "/tmp/ffbase_XXXXXXX";
  auto* result = ::mkdtemp(directory_name);
  if (result == nullptr) {
    return "";
  }
  return {result};
}

static int ToPosixAccessFlags(ffbase::FilePermission permission) {
  int mode = 0;
  switch (permission) {
  case FilePermission::kRead:
    mode |= O_RDONLY;
    break;
  case FilePermission::kWrite:
    mode |= O_WRONLY;
    break;
  case FilePermission::kReadWrite:
    mode |= O_RDWR;
    break;
  }

  return mode;
}

static int ToPosixCreateModeFlags(ffbase::FilePermission permission) {
  int mode = 0;
  switch (permission) {
  case FilePermission::kRead:
    mode |= S_IRUSR;
    break;
  case FilePermission::kWrite:
    mode |= S_IWUSR;
    break;

  case FilePermission::kReadWrite:
    mode |= S_IRUSR | S_IWUSR;
    break;
  }

  return mode;
}

ffbase::UniqueFD OpenFile(const char *path, bool need_create,
                          FilePermission permission) {
  return OpenFile(ffbase::UniqueFD{AT_FDCWD}, path, need_create, permission);
}

ffbase::UniqueFD OpenFile(const ffbase::UniqueFD& base_dir, const char* path,
                          bool need_create, FilePermission permission) {
  if (nullptr == path) { return {}; }

  int flags = 0, mode = 0;

  if (need_create && !FileExists(base_dir, path)) {
    flags =  ToPosixAccessFlags(permission) | O_CREAT | O_TRUNC;
    mode = ToPosixCreateModeFlags(permission);
  } else {
    flags = ToPosixAccessFlags(permission);
    mode = 0; //
  }

  return ffbase::UniqueFD {::openat(base_dir.get(), path, flags, mode)};

}

ffbase::UniqueFD OpenDirectory(const char *path, bool need_create,
                               FilePermission permission) {
  return OpenDirectory(ffbase::UniqueFD{AT_FDCWD}, path, need_create,
                       permission);
}

ffbase::UniqueFD OpenDirectory(const ffbase::UniqueFD &base_dir,
                               const char *path, bool need_create,
                               FilePermission permission) {
  if (nullptr == path) { return {}; }

  if (need_create && !FileExists(base_dir, path)) {
    if (::mkdirat(base_dir.get(), path, ToPosixCreateModeFlags(permission) | S_IXUSR) != 0) {
      return {};
    }
  }

  return ffbase::UniqueFD {::openat(base_dir.get(), path, O_RDONLY | O_DIRECTORY)};
}

bool IsDirectory(const ffbase::UniqueFD& dir) {
  if (!dir.is_valid()) {
    return false;
  }

  struct stat stat_result = {};

  if (::fstat(dir.get(), &stat_result) != 0) {
    return false;
  }

  return S_ISDIR(stat_result.st_mode);
}

bool IsDirectory(const ffbase::UniqueFD& dir, const char* path) {
  auto file = OpenFile(dir, path, false, FilePermission::kRead);
  return (file.is_valid() && IsDirectory(file));
}

bool IsFile(const char* path) {
  if (nullptr == path) {
    return false;
  }

  struct stat stat_result {};

  if (stat(path, &stat_result) != 0) {
    return false;
  }

  return S_ISREG(stat_result.st_mode);
}

bool TruncateFile(const ffbase::UniqueFD& file, size_t size) {
  if (!file.is_valid()) {
    return false;
  }

  return ::ftruncate(file.get(), size) == 0;
}


bool FileExists(const ffbase::UniqueFD& base_dir, const char* path) {
  if (!base_dir.is_valid()) {
    return false;
  }

  return ::faccessat(base_dir.get(), path, F_OK, 0) == 0;
}

bool UnlinkDirectory(const char* path) {
 return UnlinkDirectory(UniqueFD{AT_FDCWD}, path);
}

bool UnlinkDirectory(const ffbase::UniqueFD& base_dir, const char* path) {
  int res = unlinkat(base_dir.get(), path, AT_REMOVEDIR);
  if (res != 0) {
    FF_DLOG(ERROR) << strerror(errno);
  }

  return res == 0;
}

bool UnlinkFile(const char *path) {
  return UnlinkFile(UniqueFD{AT_FDCWD}, path);
}

bool UnlinkFile(const ffbase::UniqueFD& base_dir, const char* path) {
  int res = ::unlinkat(base_dir.get(), path, 0);
  if (res != 0) {
    FF_DLOG(ERROR) << strerror(errno);
  }

  return res == 0;
}

bool WriteAtomically(const ffbase::UniqueFD &base_dir, const char *file_name,
                     const Mapping &data) {
  if (nullptr == file_name || nullptr == data.GetMapping()) {
    return false;
  }

  std::stringstream stream;
  stream << file_name << ".tmp";

  const auto tmp_file_name = stream.str();

  auto tmp_file = OpenFile(base_dir, tmp_file_name.c_str(), true,
                           FilePermission::kReadWrite);

  if (!tmp_file.is_valid()) {
    return false;
  }

  if (!TruncateFile(tmp_file, data.GetSize())) {
    return false;
  }

  FileMapping mapping(tmp_file, {FileMapping::Protection::kWrite});

  if (mapping.GetMutableMapping() == nullptr ||
      mapping.GetSize() != data.GetSize()) {
    return false;
  }

  ::memcpy(mapping.GetMutableMapping(), data.GetMapping(), data.GetSize());

  if (::msync(mapping.GetMutableMapping(), data.GetSize(), MS_SYNC) != 0) {
    return false;
  }

  return ::renameat(base_dir.get(), tmp_file_name.c_str(), base_dir.get(),
                    file_name) == 0;
}

}// namespace ffbase