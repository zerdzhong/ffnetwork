//
// Created by zerdzhong on 2020/1/30.
//

#include "mapping.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>
#include <utility>
#include "file.h"

namespace ffbase {

static int ToPosixProtectionFlags(
    std::initializer_list<Protection> protection_flags) {
  int flags = 0;
  for (auto protection : protection_flags) {
    switch (protection) {
    case Protection::kRead:
      flags |= PROT_READ;
      break;
    case Protection::kWrite:
      flags |= PROT_WRITE;
      break;
    }
  }

  return flags;
}

static bool
IsWriteable(std::initializer_list<Protection> protection_flags) {
  for (auto protection : protection_flags) {
    if (protection == Protection::kWrite) {
      return true;
    }
  }
  return false;
}

#pragma mark- FileMapping

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

#pragma mark- AutoFileMapping

AutoSyncFileMapping::AutoSyncFileMapping(const char *path,
                                         uint block_size)
    : block_size_(block_size) {

  file_fd_ = ffbase::OpenFile(path, true, ffbase::FilePermission::kReadWrite);

  if (!file_fd_.is_valid()) { return; }

  struct stat stat_result{};

  if (::fstat(file_fd_.get(), &stat_result) != 0) {
    return;
  }

  mapping_size_ = stat_result.st_size;

  if (mapping_size_ == 0) {
    if (!GrowFileSpace()) {
      FF_LOG(ERROR) << strerror(errno);
      return ;
    }
  } else {
    TryMap();
  }

  valid_ = true;
}

AutoSyncFileMapping::~AutoSyncFileMapping() {
  if (mapping_ != nullptr) {
    SyncMap();
    ::munmap(mapping_, mapping_size_);

    ffbase::TruncateFile(file_fd_, mapping_size_ - left_space_size_);
  }
}

bool AutoSyncFileMapping::AppendData(const uint8_t *data, size_t length) {

  auto append_offset = mapping_size_ - left_space_size_;

  while (length > left_space_size_) {
    if (!GrowFileSpace()) {
      return false;
    }
  }

  ::memcpy(mapping_+append_offset, data, length);
  left_space_size_ -= length;

  return true;
}

const uint8_t *AutoSyncFileMapping::GetMapping() const { return mapping_; }
size_t AutoSyncFileMapping::GetSize() const { return mapping_size_; }

bool AutoSyncFileMapping::GrowFileSpace() {

  if (!ffbase::TruncateFile(file_fd_, mapping_size_ + block_size_)) {
    return false;
  }

  left_space_size_ += block_size_;

  if (mapping_ != nullptr) {
    SyncMap();
    ::munmap(mapping_, mapping_size_);
  }

  mapping_size_ += block_size_;

  return TryMap();
}

bool AutoSyncFileMapping::TryMap() {
  auto *mapping =
      ::mmap(nullptr, mapping_size_, ToPosixProtectionFlags({Protection::kWrite}),
             MAP_SHARED, file_fd_.get(), 0);

  if (mapping == MAP_FAILED) {
    FF_LOG(ERROR) << strerror(errno);
    return false;
  }

  mapping_ = static_cast<uint8_t*>(mapping);

  return true;
}

bool AutoSyncFileMapping::SyncMap() {
  if (mapping_ == nullptr) {
    return false;
  }

  if (::msync(mapping_, mapping_size_, MS_SYNC) == -1) {
    FF_LOG(ERROR) << strerror(errno);
    return false;
  }

  return true;
}

bool AutoSyncFileMapping::IsValid() const { return valid_; }

}//namespace ffbase
