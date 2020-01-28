//
// Created by zerdzhong on 2019/10/12.
//

#ifndef FFBASE_UNIQUE_OBJECT_H
#define FFBASE_UNIQUE_OBJECT_H

#include <utility>

#include "compiler_specific.h"
#include "logging.h"
#include "macros.h"

namespace ffbase {

template <typename T, typename Traits> class UniqueObject {
private:
  struct Data : public Traits {
    explicit Data(const T &data) : generic_data(data) {}
    Data(const T &data, const Traits other)
        : Traits(other), generic_data(data) {}

    T generic_data;
  };

public:
  using element_type = T;
  using traits_type = Traits;

  UniqueObject() : data_(Traits::InvalidValue()) {}
  explicit UniqueObject(const element_type &value) : data_(value) {}

  UniqueObject(const element_type &value, const traits_type &traits)
      : data_(value, traits) {}

  UniqueObject(UniqueObject &&other)
      : data_(other.release(), other.get_traits()) {}

  ~UniqueObject() { FreeIfNecessary(); }

  UniqueObject &operator=(UniqueObject &&other) {
    reset(other.release());
    return *this;
  }

  void reset(const T &value = Traits::InvalidValue()) {
    FF_CHECK(data_.generic_data == Traits::InvalidValue() ||
             data_.generic_data != value);
    FreeIfNecessary();
    data_.generic_data = value;
  }

  void swap(UniqueObject &other) {
    // Standard swap idiom: 'using std::swap' ensures that std::swap is
    // present in the overload set, but we call swap unqualified so that
    // any more-specific overloads can be used, if available.
    using std::swap;
    swap(static_cast<Traits &>(data_), static_cast<Traits &>(other.data_));
    swap(data_.generic_data, other.data_.generic_data);
  }

  element_type release() FF_WARN_UNUSED_RESULT {
    element_type old_generic = data_.generic_data;
    data_.generic_data = Traits::InvalidValue();
    return old_generic;
  }

  const element_type &get() const { return data_.generic_data; }

  bool is_valid() const { return Traits::IsValid(data_.generic_data); }
  bool operator==(const T &value) const { return data_.generic_data == value; }
  bool operator!=(const T &value) const { return data_.generic_data != value; }

  traits_type &get_traits() { return data_; }
  const traits_type &get_traits() const { return data_; }

private:
  void FreeIfNecessary() {
    if (data_.generic_data != Traits::InvalidValue()) {
      data_.Free(data_.generic_data);
      data_.generic_data = Traits::InvalidValue();
    }
  }

  Data data_;
  FF_DISALLOW_COPY_AND_ASSIGN(UniqueObject);
};

} // end of namespace ffbase

#endif // FFBASE_UNIQUE_OBJECT_H
