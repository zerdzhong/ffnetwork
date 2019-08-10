#include "critical_section.h"

namespace ffnetwork {
#pragma mark CriticalSection
    CriticalSection::CriticalSection() {
        pthread_mutexattr_t mutex_attribute;
        
        pthread_mutexattr_init(&mutex_attribute);
        pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mutex_, &mutex_attribute);

        pthread_mutexattr_destroy(&mutex_attribute);
    }

    CriticalSection::~CriticalSection() {
        pthread_mutex_destroy(&mutex_);
    }

    void CriticalSection::Enter() EXCLUSIVE_LOCK_FUNCTION() {
        pthread_mutex_lock(&mutex_);
    }

    bool CriticalSection::TryEnter() EXCLUSIVE_TRYLOCK_FUNCTION(true) {
        return pthread_mutex_trylock(&mutex_) == 0;
    }

    void CriticalSection::Leave() UNLOCK_FUNCTION() {
        pthread_mutex_unlock(&mutex_);
    }

#pragma mark CriticalScope
    CriticalScope::CriticalScope(CriticalSection* cs) : critical_section_(cs) { critical_section_->Enter(); }
    CriticalScope::~CriticalScope() { critical_section_->Leave(); }
}