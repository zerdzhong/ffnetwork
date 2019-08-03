#include "logger.h"

#if !LINUX && !ANDROID
#include <sys/timeb.h>
#endif

#include <sstream>
#include <unistd.h>
#include <pthread.h>

namespace ffnetwork {

static pthread_mutex_t gCommonLogMutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef DEBUG
    static ELogLevel gEnableLogLevel = ELOGLEVEL_DEBUG;
#else
    static ELogLevel gEnableLogLevel = ELOGLEVEL_ERROR;
#endif

    void set_enable_loglevel(ELogLevel enable_level) {
        pthread_mutex_lock(&gCommonLogMutex);
        gEnableLogLevel = enable_level;
        pthread_mutex_unlock(&gCommonLogMutex);
    }

    ELogLevel enable_loglevel() {
        pthread_mutex_lock(&gCommonLogMutex);
        ELogLevel enable_level = gEnableLogLevel ;
        pthread_mutex_unlock(&gCommonLogMutex);

        return enable_level;
    }

    void log(ELogLevel level, const char* log_tag, const char* module_tag, const char* format, ...) {
        if (level < enable_loglevel()) {
            return;
        }

        va_list ap;
        va_start(ap, format);
        vlog(level, log_tag, module_tag, format, ap);
        va_end(ap);
    }

    void vlog(ELogLevel level, const char* log_tag, const char* module_tag, const char* format, va_list args) {
        if (level < enable_loglevel()) {
            return;
        }

        ::std::stringstream __ss__;
#if ANDROID
        __ss__<<"["<<log_tag<<"]["<<module_tag<<"]";
        int __prio__ = get_android_log_priority(level);

        char buf[1024] = {0};
        vsnprintf(buf, 1023, format, args);

        __android_log_vprint(__prio__, __ss__.str().c_str(), format, args);
#else
        pthread_mutex_lock(&gCommonLogMutex);
        struct tm *__now;
        struct timeb __tb;
        char __datestr[16];
        char __timestr[16];
        char __mss[4];
        ftime(&__tb);
        __now=localtime(&__tb.time);
        sprintf(__datestr, "%04d-%02d-%02d", __now->tm_year+1900, __now->tm_mon+1, __now->tm_mday);
        sprintf(__timestr, "%02d:%02d:%02d", __now->tm_hour, __now->tm_min, __now->tm_sec );
        sprintf(__mss,"%03d",__tb.millitm);

        __ss__<<__datestr<<" "<<__timestr<<"."<<__mss<<" ["<<log_tag<<"]:"<<"["<<module_tag<<"]:";
        char buf[1024] = {0};
        vsnprintf(buf, 1023, format, args);
        __ss__<<buf<<"\n";
        printf("%s", __ss__.str().c_str());
        pthread_mutex_unlock(&gCommonLogMutex);
#endif
    }
}