//
// Created by zerdzhong on 2020/1/30.
//

#ifndef FFNETWORK_MAPPING_H
#define FFNETWORK_MAPPING_H

#include <string>
#include <vector>
#include <memory>
#include <initializer_list>

#include "unique_fd.h"
#include "macros.h"

namespace ffbase {

class Mapping {
public:
  Mapping() = default;
  virtual ~Mapping() = default;

  virtual size_t GetSize() const = 0;
  virtual const uint8_t* GetMapping() const = 0;

private:
  FF_DISALLOW_COPY_AND_ASSIGN(Mapping);
};

class FileMapping final : public Mapping {
public:
  enum class Protection {
    kRead,
    kWrite,
  };

  FileMapping(const ffbase::UniqueFD &fd,
              std::initializer_list<Protection> protection = {
                  Protection::kRead});

  ~FileMapping() override ;

  static std::unique_ptr<FileMapping> CreateReadOnly(const std::string &path);
  static std::unique_ptr<FileMapping>
  CreateReadOnly(const ffbase::UniqueFD &base_dir, const std::string &sub_path);

  size_t GetSize() const override ;
  const uint8_t* GetMapping() const override ;

  uint8_t* GetMutableMapping();
  bool IsValid() const;

private:
  bool valid_ = false;
  size_t size_ = 0;
  uint8_t* mapping_ = nullptr;
  uint8_t* mutable_mapping_ = nullptr;

  FF_DISALLOW_COPY_AND_ASSIGN(FileMapping);
};

class DataMapping final : public Mapping {
public:
  DataMapping(std::vector<uint8_t> data);
  DataMapping(const std::string& data);

  ~DataMapping() override ;

  size_t GetSize() const override ;
  const uint8_t* GetMapping() const override ;

private:
  std::vector<uint8_t> data_;
  FF_DISALLOW_COPY_AND_ASSIGN(DataMapping);
};

}// namespace ffbase
#endif // FFNETWORK_MAPPING_H
