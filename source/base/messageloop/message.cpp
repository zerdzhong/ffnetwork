#include "message.h"
#include "logging.h"
#include <cstring>

namespace ffbase {

static uint32_t NextOfTwoSize(uint32_t x) {
  if (x == 0) {
    return 1;
  }

  --x;

  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;

  return x + 1;
}

Message::Message() = default;
Message::~Message() = default;

#pragma mark - public_method

const uint8_t *Message::GetBuffer() const { return buffer_; }

size_t Message::GetBufferSize() const { return buffer_length_; }

size_t Message::GetDataLength() const { return data_length_; }

size_t Message::GetSizeRead() const { return size_read_; }

void Message::ResetRead() { size_read_ = 0; }

#pragma mark - private_method

bool Message::Reserve(size_t size) {
  if (buffer_length_ >= size) {
    return true;
  }

  return Resize(NextOfTwoSize(size));
}

bool Message::Resize(size_t size) {
  if (buffer_ == nullptr) {
    FF_DCHECK(buffer_length_ == 0);
    void *buffer = malloc(size);
    const bool malloc_success = (buffer != nullptr);
    if (malloc_success) {
      buffer_ = static_cast<uint8_t *>(buffer);
      buffer_length_ = size;
    }

    return malloc_success;
  }

  FF_DCHECK(size > buffer_length_);

  void *resized = realloc(buffer_, size);
  const bool success = (resized != nullptr);

  if (success) {
    buffer_ = static_cast<uint8_t *>(resized);
    buffer_length_ = size;
  }

  return success;
}

uint8_t *Message::PrepareEncode(size_t size) {
  if (!Reserve(buffer_length_ + size)) {
    return nullptr;
  }

  auto old_length = data_length_;
  data_length_ += size;
  return buffer_ + old_length;
}

uint8_t *Message::PrepareDecode(size_t size) {
  if ((size + size_read_) > buffer_length_) {
    return nullptr;
  }

  auto *buffer = buffer_ + size_read_;
  size_read_ += size;
  return buffer;
}

} // namespace ffbase
