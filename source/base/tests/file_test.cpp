//
// Created by zerdzhong on 2020/1/30.
//

#include "gtest/gtest.h"
#include "file.h"
#include "mapping.h"

using namespace ffbase;

static bool WriteStringToFile(const ffbase::UniqueFD& fd,
                              const std::string& contents) {
  if (!ffbase::TruncateFile(fd, contents.size())) {
    return false;
  }

  ffbase::FileMapping mapping(fd, {ffbase::FileMapping::Protection::kWrite});
  if (mapping.GetSize() != contents.size()) {
    return false;
  }

  if (mapping.GetMutableMapping() == nullptr) {
    return false;
  }

  ::memmove(mapping.GetMutableMapping(), contents.data(), contents.size());
  return true;
}

static std::string ReadStringFromFile(const ffbase::UniqueFD& fd) {
  ffbase::FileMapping mapping(fd);

  if (mapping.GetMapping() == nullptr) {
    return nullptr;
  }

  return {reinterpret_cast<const char*>(mapping.GetMapping()),
          mapping.GetSize()};
}

TEST(FileTest, CreateTemporaryAndUnlink) {
  auto dir_name = ffbase::CreateTemporaryDirectory();
  ASSERT_NE(dir_name, "");
  auto dir =
      ffbase::OpenDirectory(dir_name.c_str(), false, ffbase::FilePermission::kRead);
  ASSERT_TRUE(dir.is_valid());
  dir.reset();
  ASSERT_TRUE(ffbase::UnlinkDirectory(dir_name.c_str()));
}

TEST(FileTest, ScopedTempDirIsValid) {
  ffbase::ScopedTemporaryDirectory dir;
  ASSERT_TRUE(dir.fd().is_valid());
}

TEST(FileTest, CanOpenFileForWriting) {
  ffbase::ScopedTemporaryDirectory dir;
  ASSERT_TRUE(dir.fd().is_valid());

  auto fd =
      ffbase::OpenFile(dir.fd(), "some.txt", true, ffbase::FilePermission::kWrite);
  ASSERT_TRUE(fd.is_valid());
  fd.reset();
  ASSERT_TRUE(ffbase::UnlinkFile(dir.fd(), "some.txt"));
}

TEST(FileTest, CanTruncateAndWrite) {
  ffbase::ScopedTemporaryDirectory dir;
  ASSERT_TRUE(dir.fd().is_valid());

  std::string contents = "some contents here";

  {
    auto fd = ffbase::OpenFile(dir.fd(), "some.txt", true,
                            ffbase::FilePermission::kReadWrite);
    ASSERT_TRUE(fd.is_valid());

    ASSERT_TRUE(ffbase::TruncateFile(fd, contents.size()));

    ffbase::FileMapping mapping(fd, {ffbase::FileMapping::Protection::kWrite});
    ASSERT_EQ(mapping.GetSize(), contents.size());
    ASSERT_NE(mapping.GetMutableMapping(), nullptr);

    ::memcpy(mapping.GetMutableMapping(), contents.data(), contents.size());
  }

  {
    auto fd =
        ffbase::OpenFile(dir.fd(), "some.txt", false, ffbase::FilePermission::kRead);
    ASSERT_TRUE(fd.is_valid());

    ffbase::FileMapping mapping(fd);
    ASSERT_EQ(mapping.GetSize(), contents.size());

    ASSERT_EQ(0,
              ::memcmp(mapping.GetMapping(), contents.data(), contents.size()));
  }

  ffbase::UnlinkFile(dir.fd(), "some.txt");
}

TEST(FileTest, CreateDirectoryStructure) {
  ffbase::ScopedTemporaryDirectory dir;

  std::string contents = "These are my contents";
  {
    auto sub = ffbase::CreateDirectory(dir.fd(), {"a", "b", "c"},
                                    ffbase::FilePermission::kReadWrite);
    ASSERT_TRUE(sub.is_valid());
    auto file = ffbase::OpenFile(sub, "my_contents", true,
                              ffbase::FilePermission::kReadWrite);
    ASSERT_TRUE(file.is_valid());
    ASSERT_TRUE(WriteStringToFile(file, contents));
  }

  const char* file_path = "a/b/c/my_contents";

  {
    auto contents_file =
        ffbase::OpenFile(dir.fd(), file_path, false, ffbase::FilePermission::kRead);
    ASSERT_EQ(ReadStringFromFile(contents_file), contents);
  }

  // Cleanup.
  ASSERT_TRUE(ffbase::UnlinkFile(dir.fd(), file_path));
  ASSERT_TRUE(ffbase::UnlinkDirectory(dir.fd(), "a/b/c"));
  ASSERT_TRUE(ffbase::UnlinkDirectory(dir.fd(), "a/b"));
  ASSERT_TRUE(ffbase::UnlinkDirectory(dir.fd(), "a"));
}

#if OS_WIN
#define AtomicWriteTest DISABLED_AtomicWriteTest
#else
#define AtomicWriteTest AtomicWriteTest
#endif
TEST(FileTest, AtomicWriteTest) {
  ffbase::ScopedTemporaryDirectory dir;

  const std::string contents = "These are my contents.";

  auto data = std::make_unique<ffbase::DataMapping>(
      std::vector<uint8_t>{contents.begin(), contents.end()});

  // Write.
  ASSERT_TRUE(ffbase::WriteAtomically(dir.fd(), "precious_data", *data));

  // Read and verify.
  ASSERT_EQ(contents,
            ReadStringFromFile(ffbase::OpenFile(dir.fd(), "precious_data", false,
                                             ffbase::FilePermission::kRead)));

  // Cleanup.
  ASSERT_TRUE(ffbase::UnlinkFile(dir.fd(), "precious_data"));
}

TEST(FileTest, EmptyMappingTest) {
  ffbase::ScopedTemporaryDirectory dir;

  {
    auto file = ffbase::OpenFile(dir.fd(), "my_contents", true,
                              ffbase::FilePermission::kReadWrite);

    ffbase::FileMapping mapping(file);
    ASSERT_TRUE(mapping.IsValid());
    ASSERT_EQ(mapping.GetSize(), 0ul);
    ASSERT_EQ(mapping.GetMapping(), nullptr);
  }

  ASSERT_TRUE(ffbase::UnlinkFile(dir.fd(), "my_contents"));
}
