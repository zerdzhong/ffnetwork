#ifndef FFBASE_MESSAGE_H
#define FFBASE_MESSAGE_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>
#include <memory>
#include <type_traits>

#include "compiler_specific.h"
#include "macros.h"

namespace ffbase {

class Message;

class MessageSerializable {
    public:
        virtual ~MessageSerializable() = default;

        virtual bool Serialize(Message& message) const = 0;

        virtual bool Deserialize(Message& message) = 0;

        virtual size_t GetSerializableTag() const { return 0; };
};

class Message {
public:
    Message();
    ~Message();

    const uint8_t* GetBuffer() const;
    size_t GetBufferSize() const;
    size_t GetDataLength() const;
    size_t GetSizeRead() const;

    void ResetRead();

    //Encoder
    template <typename T, typename = std::enable_if<std::is_trivially_copyable<T>::value>>
    FF_WARN_UNUSED_RESULT bool Encode(const T& value) {
        if (auto* buffer = PrepareEncode(sizeof(T))) {
            memcpy(buffer, &value, sizeof(T));
            return true;
        }
        
        return false;
    }
    
    FF_WARN_UNUSED_RESULT bool Encode(const MessageSerializable& value) {
        return value.Serialize(*this);
    }
    
    //Decoder
    template <typename T, typename = std::enable_if<std::is_trivially_copyable<T>::value>>
    FF_WARN_UNUSED_RESULT bool Decode(T& value) {
        if (auto* buffer = PrepareDecode(sizeof(T))) {
            memcpy(&value, buffer, sizeof(T));
            return true;
        }
        
        return false;
    }

private:
    uint8_t* buffer_ = nullptr;
    size_t buffer_length_ = 0;
    size_t data_length_ = 0;
    size_t size_read_ = 0;
    
    FF_WARN_UNUSED_RESULT bool Reserve(size_t size);
    FF_WARN_UNUSED_RESULT bool Resize(size_t size);
    FF_WARN_UNUSED_RESULT uint8_t* PrepareEncode(size_t size);
    FF_WARN_UNUSED_RESULT uint8_t* PrepareDecode(size_t size);
    
    FF_DISALLOW_COPY_AND_ASSIGN(Message);
};

} //end of namespace of ffbase

#endif //FFBASE_MESSAGE_H
