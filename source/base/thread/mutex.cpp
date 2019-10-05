//
// Created by zerdzhong on 2019/10/5.
//

#include "mutex.h"
#include <pthread.h>

namespace ffbase {

class SharedMutexPosix : public SharedMutex {
public:
    SharedMutexPosix() {
        pthread_rwlock_init(&rw_lock_, nullptr);
    }
    
    ~SharedMutexPosix() {
        pthread_rwlock_destroy(&rw_lock_);
    }
    
    void Lock() FF_ACQUIRE() override;
    void LockShared() FF_ACQUIRE_SHARED() override;
    
    // Release/unlock an exclusive mutex.
    void Unlock() FF_RELEASE() override;
    // Release/unlock a shared mutex.
    void UnlockShared() FF_RELEASE_SHARED() override;
    
    // Try to acquire the mutex.  Returns true on success, and false on failure.
    bool TryLock() FF_TRY_ACQUIRE(true) override;
    // Try to acquire the mutex for read operations.
    bool TryLockShared() FF_TRY_ACQUIRE_SHARED(true) override;
    
private:
     pthread_rwlock_t rw_lock_;
};

SharedMutex* SharedMutex::Create() {
  return new SharedMutexPosix();
}

void SharedMutexPosix::Lock() {
    pthread_rwlock_wrlock(&rw_lock_);
}

void SharedMutexPosix::LockShared() {
    pthread_rwlock_rdlock(&rw_lock_);
}

void SharedMutexPosix::Unlock() {
    pthread_rwlock_unlock(&rw_lock_);
}

void SharedMutexPosix::UnlockShared() {
  pthread_rwlock_unlock(&rw_lock_);
}

bool SharedMutexPosix::TryLock() {
    return pthread_rwlock_trywrlock(&rw_lock_);
}

bool SharedMutexPosix::TryLockShared() {
    return pthread_rwlock_tryrdlock(&rw_lock_);
}

}
