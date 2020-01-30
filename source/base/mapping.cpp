//
// Created by zerdzhong on 2020/1/30.
//

#include <memory>
#include "mapping.h"
#include "file.h"

namespace ffbase {

#pragma mark- FileMapping

uint8_t *FileMapping::GetMutableMapping() { return mutable_mapping_; }

std::unique_ptr<FileMapping>
FileMapping::CreateReadOnly(const std::string &path) {
  return CreateReadOnly(OpenFile(path.c_str(), false, FilePermission::kRead),
                        "");
}

std::unique_ptr<FileMapping>
FileMapping::CreateReadOnly(const ffbase::UniqueFD &base_fd,
                            const std::string &sub_path) {
  if (!sub_path.empty()) {
    return CreateReadOnly(
        OpenFile(base_fd, sub_path.c_str(), false, FilePermission::kRead), "");
  }

  auto mapping = std::unique_ptr<FileMapping>(new FileMapping(
      base_fd, std::initializer_list<Protection>{Protection::kRead}));

  if (!mapping->IsValid()) {
    return nullptr;
  }

  return mapping;
}

#pragma mark- DataMapping

DataMapping::DataMapping(std::vector<uint8_t> data) : data_(std::move(data)) {}

DataMapping::DataMapping(const std::string& string)
    : data_(string.begin(), string.end()) {}

DataMapping::~DataMapping() = default;

size_t DataMapping::GetSize() const {
  return data_.size();
}

const uint8_t* DataMapping::GetMapping() const {
  return data_.data();
}

}//namespace ffbase
