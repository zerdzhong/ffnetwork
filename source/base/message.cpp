#include "message.h"

namespace ffbase {

Message::Message() = default;
Message::~Message() = default;

const uint8_t* Message::GetBuffer() const {
    return buffer_;
}

size_t Message::GetBufferSize() const {
    return buffer_length_;
}

size_t Message::GetDataLength() const {
    return data_length_;
}

size_t Message::GetSizeRead() const {
    return size_read_;
}

void Message::ResetRead() {
    size_read_ = 0;
}


} //end if namespace ffbase
