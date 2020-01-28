//
// Created by zerdzhong on 2019/10/11.
//

#ifndef FFBASE_CF_REFERENCE_UTILS_H
#define FFBASE_CF_REFERENCE_UTILS_H

#include "macros.h"
#include <CoreFoundation/CoreFoundation.h>

namespace ffbase {

template <class T> class CFRef {
public:
  CFRef(T instance) : instance_(instance) {}
  CFRef() : instance_(nullptr) {}

  ~CFRef() {
    if (instance_ != nullptr) {
      CFRelease(instance_);
    }

    instance_ = nullptr;
  }

  void Reset(T instance) {
    if (instance_ == instance) {
      return;
    }
    if (instance_ != nullptr) {
      CFRelease(instance_);
    }

    instance_ = instance;
  }

  operator T() const { return instance_; }

  operator bool() const { return instance_ != nullptr; }

private:
  T instance_;
  FF_DISALLOW_COPY_AND_ASSIGN(CFRef);
};

} // namespace ffbase

#endif // FFBASE_CF_REFERENCE_UTILS_H
