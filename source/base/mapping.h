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

enum class Protection {
  kRead,
  kWrite,
};

class FileMapping final : public Mapping {
public:
  explicit FileMapping(const ffbase::UniqueFD &fd,
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

class AutoSyncFileMapping final : public Mapping {
public:
  explicit AutoSyncFileMapping(const char *path,
                               uint block_size = 10 * 1024 * 1024);

  ~AutoSyncFileMapping() override ;

  size_t GetSize() const override ;
  const uint8_t* GetMapping() const override ;

  bool AppendData(const uint8_t* data, size_t length);
  bool IsValid() const;

private:
  bool GrowFileSpace();
  bool SyncMap();
  bool TryMap();

private:
  bool valid_ = false;
  uint block_size_ = 10 * 1024 * 1024;
  uint8_t* mapping_ = nullptr;
  size_t mapping_size_ = 0;
  size_t left_space_size_ = 0;
  ffbase::UniqueFD file_fd_;

  FF_DISALLOW_COPY_AND_ASSIGN(AutoSyncFileMapping);
};

class DataMapping final : public Mapping {
public:
  explicit DataMapping(std::vector<uint8_t> data);
  explicit DataMapping(const std::string& data);

  ~DataMapping() override ;

  size_t GetSize() const override ;
  const uint8_t* GetMapping() const override ;

private:
  std::vector<uint8_t> data_;
  FF_DISALLOW_COPY_AND_ASSIGN(DataMapping);
};

}// namespace ffbase
#endif // FFNETWORK_MAPPING_H
