//
// Created by zerdzhong on 2019/10/6.
//

#ifndef FFNETWORK_THREAD_LOCAL_H
#define FFNETWORK_THREAD_LOCAL_H

#include "macros.h"
#include <pthread.h>

namespace ffbase {

#define FF_THREAD_LOCAL static

namespace internal {
class ThreadLocalPointer {
public:
  ThreadLocalPointer(void (*destroy)(void *));
  ~ThreadLocalPointer();

  void *get() const;
  void *swap(void *ptr);

private:
  pthread_key_t key_;
  FF_DISALLOW_COPY_AND_ASSIGN(ThreadLocalPointer);
};
} // namespace internal

template <typename T> class ThreadLocalUniquePtr {
public:
  ThreadLocalUniquePtr() : ptr_(destroy) {}

  T *get() const { return reinterpret_cast<T *>(ptr_.get()); }
  void reset(T *ptr) { destroy(ptr_.swap(ptr)); }

private:
  static void destroy(void *ptr) { delete reinterpret_cast<T *>(ptr); }

  internal::ThreadLocalPointer ptr_;

  FF_DISALLOW_COPY_AND_ASSIGN(ThreadLocalUniquePtr);
};

} // namespace ffbase

#endif // FFNETWORK_THREAD_LOCAL_H
