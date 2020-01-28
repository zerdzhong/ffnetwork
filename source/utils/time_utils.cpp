#include "time_utils.h"
#include <chrono>

namespace ffnetwork {

const uint32_t HALF = 0x80000000;

uint32_t NowTimeMillis() {
  return static_cast<uint32_t>(NowTimeNanos() / kNumNanosecsPerMillisec);
}

uint64_t NowTimeMicros() {
  return static_cast<uint64_t>(NowTimeNanos() / kNumNanosecsPerMicrosec);
}

uint64_t NowTimeNanos() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

uint32_t TimeAfter(int32_t elapsed) { return NowTimeMillis() + elapsed; }

bool TimeIsBetween(uint32_t earlier, uint32_t middle, uint32_t later) {
  if (earlier <= later) {
    return ((earlier <= middle) && (middle <= later));
  } else {
    return !((later < middle) && (middle < earlier));
  }
}

bool TimeIsLaterOrEqual(uint32_t earlier, uint32_t later) {
  int32_t diff = later - earlier;
  return (diff >= 0 && static_cast<uint32_t>(diff) < HALF);
}

bool TimeIsLater(uint32_t earlier, uint32_t later) {
  int32_t diff = later - earlier;
  return (diff > 0 && static_cast<uint32_t>(diff) < HALF);
}

int32_t TimeDiff(uint32_t later, uint32_t earlier) { return later - earlier; }

} // namespace ffnetwork
