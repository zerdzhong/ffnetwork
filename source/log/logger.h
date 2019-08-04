//
// Created by zerdzhong on 2019-08-03.
//

#ifndef FFNETWORK_LOGGER_H
#define FFNETWORK_LOGGER_H

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef ANDROID
#define log_printflike(fmtarg, firstvararg) \
__attribute__((__format__ (printf, fmtarg, firstvararg)))
#else
#define log_printflike(fmtarg, firstvararg) __attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#endif

namespace ffnetwork {

    enum ELogLevel {
        ELOGLEVEL_INFO= 0,
        ELOGLEVEL_DEBUG,
        ELOGLEVEL_WARNING,
        ELOGLEVEL_ERROR,
    };

    void log(ELogLevel level, const char* log_tag, const char* module_tag, const char* format, ...) log_printflike(4, 5);
    void vlog(ELogLevel level, const char* log_tag, const char* module_tag, const char* format, va_list args) log_printflike(4, 0);

    void set_enable_loglevel(ELogLevel enable_level);
    ELogLevel enable_loglevel();
}


#endif //FFNETWORK_LOGGER_H