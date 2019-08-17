//
// Created by zhongzhendong on 2019-08-17.
//

#ifndef FFNETWORK_CONSTRUCT_MACRO_H
#define FFNETWORK_CONSTRUCT_MACRO_H

// Put this in the declarations for a class to be unassignable.
#define DISALLOW_ASSIGN(TypeName) \
  void operator=(const TypeName&) = delete
// A macro to disallow the copy constructor and operator= functions. This should
// be used in the declarations for a class.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;          \
  DISALLOW_ASSIGN(TypeName)
// A macro to disallow all the implicit constructors, namely the default
// constructor, copy constructor and operator= functions.
//
// This should be used in the declarations for a class that wants to prevent
// anyone from instantiating it. This is especially useful for classes
// containing only static methods.

#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName() = delete;                               \
  DISALLOW_COPY_AND_ASSIGN(TypeName)

#endif //FFNETWORK_CONSTRUCT_MACRO_H
