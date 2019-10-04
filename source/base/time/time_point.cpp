//
// Created by zerdzhong on 2019/10/4.
//

#include "time_point.h"

namespace ffbase {
    TimePoint TimePoint::Now() {
        const auto elapsed_time = std::chrono::steady_clock::now().time_since_epoch();
        return TimePoint(
                std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_time)
                        .count());
    }
}