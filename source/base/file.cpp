//
// Created by zerdzhong on 2020/1/30.
//

#include "file.h"
#include "logging.h"

namespace ffbase {

ffbase::UniqueFD CreateDirectory(const ffbase::UniqueFD &base_dir,
                                 const std::vector<std::string>& components,
                                 FilePermission permission,
                                 size_t index) {
  FF_DCHECK(index < components.size());

  const char* file_path = components[index].c_str();

  auto dir = OpenDirectory(base_dir, file_path, true, permission);

  if (!dir.is_valid()) {
    return {};
  }

  if (index == components.size() - 1) {
    return dir;
  }

  return CreateDirectory(dir, components, permission, index + 1);
}

ffbase::UniqueFD CreateDirectory(const ffbase::UniqueFD &base_dir,
                                 const std::vector<std::string>& components,
                                 FilePermission permission) {
  if (!IsDirectory(base_dir) || components.empty()) {
    return {};
  }

  return CreateDirectory(base_dir, components, permission, 0);
}

#pragma mark- ScopedTemporaryDirectory

ScopedTemporaryDirectory::ScopedTemporaryDirectory() {
  path_ = CreateTemporaryDirectory();
  if (!path_.empty()) {
    dir_fd_ = OpenDirectory(path_.c_str(), false, FilePermission::kRead);
  }
}

ScopedTemporaryDirectory::~ScopedTemporaryDirectory() {
  // Windows has to close UniqueFD first before UnlinkDirectory
  dir_fd_.reset();
  if (!path_.empty()) {
    if (!UnlinkDirectory(path_.c_str())) {
      FF_LOG(ERROR) << "Could not remove directory: " << path_;
    }
  }
}

}// namespace ffbase
