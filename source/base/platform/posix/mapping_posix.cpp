//
// Created by zerdzhong on 2020/1/30.
//

#include "mapping.h"
#include <sys/mman.h>
#include <sys/stat.h>

namespace ffbase {

static int ToPosixProtectionFlags(
    std::initializer_list<FileMapping::Protection> protection_flags) {
  int flags = 0;
  for (auto protection : protection_flags) {
    switch (protection) {
    case FileMapping::Protection::kRead:
      flags |= PROT_READ;
      break;
    case FileMapping::Protection::kWrite:
      flags |= PROT_WRITE;
      break;
    }
  }

  return flags;
}

static bool
IsWriteable(std::initializer_list<FileMapping::Protection> protection_flags) {
  for (auto protection : protection_flags) {
    if (protection == FileMapping::Protection::kWrite) {
      return true;
    }
  }
  return false;
}

FileMapping::FileMapping(const ffbase::UniqueFD &fd,
                         std::initializer_list<Protection> protection)
    : size_(0), mapping_(nullptr) {
  if (!fd.is_valid()) { return; }

  struct stat stat_result{};

  if (::fstat(fd.get(), &stat_result) != 0) {
    return;
  }

  if (stat_result.st_size == 0) {
    valid_ = true;
    return;
  }

  const bool is_writeable = IsWriteable(protection);

  auto *mapping =
      ::mmap(nullptr, stat_result.st_size, ToPosixProtectionFlags(protection),
             is_writeable ? MAP_SHARED : MAP_PRIVATE, fd.get(), 0);

  if (mapping == MAP_FAILED) {
    FF_LOG(ERROR) << strerror(errno);
    return ;
  }

  mapping_ = static_cast<uint8_t*>(mapping);
  size_ = stat_result.st_size;
  valid_ = true;

  if (is_writeable) {
    mutable_mapping_ = mapping_;
  }
}

FileMapping::~FileMapping() {
  if (mapping_ != nullptr) {
    ::munmap(mapping_, size_);
  }
}

size_t FileMapping::GetSize() const {
  return size_;
}

const uint8_t* FileMapping::GetMapping() const {
  return mapping_;
}

bool FileMapping::IsValid() const {
  return valid_;
}

}//namespace ffbase
