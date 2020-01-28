#ifndef FFBASE_MACROS_H
#define FFBASE_MACROS_H

#define FF_DISALLOW_COPY(TypeName) TypeName(const TypeName &) = delete

#define FF_DISALLOW_ASSIGN(TypeName)                                           \
  TypeName &operator=(const TypeName &) = delete

#define FF_DISALLOW_MOVE(TypeName)                                             \
  TypeName(TypeName &&) = delete;                                              \
  TypeName &operator=(TypeName &&) = delete

#define FF_DISALLOW_COPY_AND_ASSIGN(TypeName)                                  \
  FF_DISALLOW_COPY(TypeName);                                                  \
  FF_DISALLOW_ASSIGN(TypeName)

#define FF_DISALLOW_COPY_ASSIGN_AND_MOVE(TypeName)                             \
  FF_DISALLOW_COPY_AND_ASSIGN(TypeName);                                       \
  FF_DISALLOW_MOVE(TypeName)

#define FF_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName)                            \
  TypeName() = delete;                                                         \
  FF_DISALLOW_COPY_ASSIGN_AND_MOVE(TypeName)

#endif // FFBASE_MACROS_H
