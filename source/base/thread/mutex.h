//
// Created by zerdzhong on 2019/10/5.
//

#ifndef FFBASE_MUTEX_H
#define FFBASE_MUTEX_H

#include "thread_annotations.h"

namespace ffbase {

class FF_CAPABILITY("mutex") SharedMutex {
public:
    static SharedMutex* Create();
    virtual ~SharedMutex() = default;
    
    virtual void Lock() FF_ACQUIRE() = 0;
    virtual void LockShared() FF_ACQUIRE_SHARED() = 0;
    
    // Release/unlock an exclusive mutex.
    virtual void Unlock() FF_RELEASE() = 0;
    // *Release/unlock a shared mutex.
    virtual void UnlockShared() FF_RELEASE_SHARED() = 0;
    
    // Try to acquire the mutex.  Returns true on success, and false on failure.
    virtual bool TryLock() FF_TRY_ACQUIRE(true) = 0;
    // Try to acquire the mutex for read operations.
    virtual bool TryLockShared() FF_TRY_ACQUIRE_SHARED(true) = 0;
};

// RAII wrapper that does a shared acquire of a SharedMutex.
class FF_SCOPED_CAPABILITY SharedLock {
public:
    explicit SharedLock(SharedMutex& shared_mutex) FF_ACQUIRE(shared_mutex) : shared_mutex_(shared_mutex) {
        shared_mutex_.LockShared();
    }
    
    ~SharedLock() FF_RELEASE() { shared_mutex_.UnlockShared(); }
    
private:
    SharedMutex& shared_mutex_;
};

// RAII wrapper that does an exclusive acquire of a SharedMutex.
class FF_SCOPED_CAPABILITY UniqueLock {
 public:
  explicit UniqueLock(SharedMutex& shared_mutex) FF_ACQUIRE(shared_mutex) : shared_mutex_(shared_mutex) {
    shared_mutex_.Lock();
  }

  ~UniqueLock() FF_RELEASE() { shared_mutex_.Unlock(); }

 private:
  SharedMutex& shared_mutex_;
};

}

#endif //FFBASE_MUTEX_H
