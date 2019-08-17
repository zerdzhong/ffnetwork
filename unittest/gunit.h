//
// Created by zhongzhendong on 2019-08-17.
//

#ifndef FFNETWORK_GUNIT_H
#define FFNETWORK_GUNIT_H

#include <thread/thread.h>
#include <utils/time_utils.h>

// Wait until "ex" is true, or "timeout" expires.
#define WAIT(ex, timeout)                                                     \
  for (uint32_t start = ffnetwork::NowTimeMillis(); !(ex) && ffnetwork::NowTimeMillis() < start + timeout;) \
    ffnetwork::Thread::Current()->ProcessMessages(1);
// This returns the result of the test in res, so that we don't re-evaluate
// the expression in the XXXX_WAIT macros below, since that causes problems
// when the expression is only true the first time you check it.
#define WAIT_(ex, timeout, res)                     \
  do {                                              \
    uint32_t start = ffnetwork::NowTimeMillis();    \
    res = (ex);                                     \
    while (!res && ffnetwork::NowTimeMillis() < start + timeout) { \
      ffnetwork::Thread::Current()->ProcessMessages(1);   \
      res = (ex);                                   \
    }                                               \
  } while (0);
// The typical EXPECT_XXXX and ASSERT_XXXXs, but done until true or a timeout.
#define EXPECT_TRUE_WAIT(ex, timeout) \
  do { \
    bool res; \
    WAIT_(ex, timeout, res); \
    if (!res) EXPECT_TRUE(ex); \
  } while (0);

#endif //FFNETWORK_GUNIT_H
