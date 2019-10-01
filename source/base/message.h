#ifndef FFBASE_MESSAGE_H
#define FFBASE_MESSAGE_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>

#include "compiler_specific.h"
#include "macros.h"

namespace ffbase {

class Message {
public:
    Message();
    ~Message();

    const uint8_t* GetBuffer() const;
    size_t GetBufferSize() const;
    size_t GetDataLength() const;
    size_t GetSizeRead() const;

    void ResetRead();

private:
    uint8_t* buffer_ = nullptr;
    size_t buffer_length_ = 0;
    size_t data_length_ = 0;
    size_t size_read_ = 0;

};

} //end of namespace of ffbase

#endif //FFBASE_MESSAGE_H
