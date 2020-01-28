//
// Created by zerdzhong on 2019/10/5.
//

// Macros for static thread-safety analysis.
//
// These are from http://clang.llvm.org/docs/ThreadSafetyAnalysis.html

#ifndef FFBASE_THREAD_ANNOTATIONS_H
#define FFBASE_THREAD_ANNOTATIONS_H

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define FF_THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define FF_THREAD_ANNOTATION_ATTRIBUTE__(x) // no-op
#endif

#define FF_CAPABILITY(x) FF_THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define FF_SCOPED_CAPABILITY FF_THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define FF_GUARDED_BY(x) FF_THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define FF_PT_GUARDED_BY(x) FF_THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define FF_ACQUIRED_BEFORE(...)                                                \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define FF_ACQUIRED_AFTER(...)                                                 \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define FF_REQUIRES(...)                                                       \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define FF_REQUIRES_SHARED(...)                                                \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define FF_ACQUIRE(...)                                                        \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define FF_ACQUIRE_SHARED(...)                                                 \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define FF_RELEASE(...)                                                        \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define FF_RELEASE_SHARED(...)                                                 \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define FF_TRY_ACQUIRE(...)                                                    \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define FF_TRY_ACQUIRE_SHARED(...)                                             \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define FF_EXCLUDES(...)                                                       \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define FF_ASSERT_CAPABILITY(x)                                                \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define FF_ASSERT_SHARED_CAPABILITY(x)                                         \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define FF_RETURN_CAPABILITY(x)                                                \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define FF_NO_THREAD_SAFETY_ANALYSIS                                           \
  FF_THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

#endif // FFBASE_THREAD_ANNOTATIONS_H
