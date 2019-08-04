#ifndef FFNETWORK_CRITICAL_SECTION_H
#define FFNETWORK_CRITICAL_SECTION_H


#include <pthread.h>
#include "thread_annotations.h"

namespace ffnetwork {
    class LOCKABLE CriticalSection {
    public:
        CriticalSection();
        ~CriticalSection();

        void Enter() EXCLUSIVE_LOCK_FUNCTION();
        bool TryEnter() EXCLUSIVE_TRYLOCK_FUNCTION(true);
        void Leave() UNLOCK_FUNCTION();

    private:
        pthread_mutex_t mutex_;
    };

    class SCOPED_LOCKABLE CriticalScope {
    public:
        explicit CriticalScope(CriticalSection* cs) EXCLUSIVE_LOCK_FUNCTION(cs);
        ~CriticalScope() UNLOCK_FUNCTION();
    private:
        CriticalSection* const critical_section_;
    };
}

#endif