//
// Created by zerdzhong on 2019/10/6.
//

#include "thread_local.h"
#include "logging.h"

namespace ffbase {
namespace internal {

ThreadLocalPointer::ThreadLocalPointer(void(*destroy)(void*)) {
    FF_CHECK(pthread_key_create(&key_, destroy) == 0);
}

ThreadLocalPointer::~ThreadLocalPointer() {
    FF_CHECK(pthread_key_delete(key_) == 0);
}

void* ThreadLocalPointer::get() const {
    return pthread_getspecific(key_);
}

void* ThreadLocalPointer::swap(void *ptr) {
    void* old_ptr = get();
    
    FF_CHECK(pthread_setspecific(key_, ptr) == 0);
    
    return old_ptr;
}

} //namespace of internal
} //namespace of ffbase
