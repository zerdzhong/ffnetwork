//
// Created by zerdzhong on 2019-08-10.
//

#ifndef FFNETWORK_EVENT_H
#define FFNETWORK_EVENT_H

#include <pthread.h>

namespace ffnetwork {
    class Event {
    public:
        static const int kForever = -1;

        Event(bool manual_reset, bool initially_signaled);
        ~Event();

        void Set();
        void Reset();

        // Wait for the event to become signaled, for the specified number of
        // |milliseconds|.  To wait indefinetly, pass kForever.
        bool Wait(int milliseconds);

    private:
        pthread_mutex_t event_mutex_;
        pthread_cond_t event_cond_;
        const bool is_manual_reset_;
        bool event_status_;
    };
}

#endif //FFNETWORK_EVENT_H
