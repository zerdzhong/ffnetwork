#ifndef FFBASE_COMPILER_SPECIFIC_H
#define FFBASE_COMPILER_SPECIFIC_H

#if !defined(__GNUC__) && !defined(__clang__) && !defined(_MSC_VER)
#error Unsupported compiler.
#endif

#define FF_ALLOW_UNUSED_LOCAL(x) false ? void(x) : void(0)

#if defined(__GNUC__) || defined(__clang__)
    #define FF_ALLOW_UNUSED_TYPE __attribute__((unused))
#else
    #define FF_ALLOW_UNUSED_TYPE
#endif

#if defined(__GUNC__) || defined(__clang__)
    #define FF_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
    #define FF_NOINLINE __declspec(noinline)
#endif

#if defined(__GUNC__) || defined(__clang__)
    #define FF_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
    #define FF_WARN_UNUSED_RESULT
#endif

#if defined(__GUNC__) || defined(__clang__)
    #define FF_PRINTF_FORMAT(format_param, dots_param)  \
        __attribute__((format(printf, format_param, dots_param)))
#else
    #define FF_PRINTF_FORMAT(format_param, dots_param)
#endif

#endif //FFBASE_COMPILER_SPECIFIC_H
