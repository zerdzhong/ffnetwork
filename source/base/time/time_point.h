//
// Created by zerdzhong on 2019/10/4.
//

#ifndef FFBASE_TIME_POINT_H
#define FFBASE_TIME_POINT_H

#include "time_delta.h"
#include <cstdint>
#include <limits>

namespace ffbase {

// A TimePoint represents a point in time represented as an integer number of
// nanoseconds elapsed since an arbitrary point in the past.

class TimePoint {
public:
  constexpr TimePoint() = default;
  static TimePoint Now();

  static constexpr TimePoint Min() {
    return TimePoint(std::numeric_limits<int64_t>::min());
  }

  static constexpr TimePoint Max() {
    return TimePoint(std::numeric_limits<int64_t>::max());
  }

  static constexpr TimePoint FromEpochDelta(TimeDelta ticks) {
    return TimePoint(ticks.ToNanoseconds());
  }

  TimeDelta ToEpochDelta() const { return TimeDelta::FromNanoseconds(ticks_); }

  // Compute the difference between two time points.
  TimeDelta operator-(TimePoint other) const {
    return TimeDelta::FromNanoseconds(ticks_ - other.ticks_);
  }

  TimePoint operator+(TimeDelta duration) const {
    return TimePoint(ticks_ + duration.ToNanoseconds());
  }
  TimePoint operator-(TimeDelta duration) const {
    return TimePoint(ticks_ - duration.ToNanoseconds());
  }

  bool operator==(TimePoint other) const { return ticks_ == other.ticks_; }
  bool operator!=(TimePoint other) const { return ticks_ != other.ticks_; }
  bool operator<(TimePoint other) const { return ticks_ < other.ticks_; }
  bool operator<=(TimePoint other) const { return ticks_ <= other.ticks_; }
  bool operator>(TimePoint other) const { return ticks_ > other.ticks_; }
  bool operator>=(TimePoint other) const { return ticks_ >= other.ticks_; }

private:
  explicit constexpr TimePoint(int64_t ticks) : ticks_(ticks) {}
  int64_t ticks_ = 0;
};

} // namespace ffbase
#endif // FFBASE_TIME_POINT_H
