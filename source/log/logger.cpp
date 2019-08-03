#include "logger.h"

#if !LINUX && !ANDROID
#include <sys/timeb.h>
#endif

#include <sstream>
#include <unistd.h>
#include <pthread.h>
#include <vector>

namespace ffnetwork {

static pthread_mutex_t gCommonLogMutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef Debug
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
        struct tm *now_time;
        struct timeb tb{};
        char date_str[16];
        char time_str[16];
        char ms_str[4];
        ftime(&tb);
        now_time=localtime(&tb.time);
        sprintf(date_str, "%04d-%02d-%02d", now_time->tm_year + 1900, now_time->tm_mon + 1, now_time->tm_mday);
        sprintf(time_str, "%02d:%02d:%02d", now_time->tm_hour, now_time->tm_min, now_time->tm_sec );
        sprintf(ms_str, "%03d", tb.millitm);

        __ss__ << date_str << " " << time_str << "." << ms_str << " [" << log_tag << "]:" << "[" << module_tag << "]:";
        
        auto temp = std::vector<char> {};
        auto length = std::size_t {63};
        while (temp.size() <= length) {
            temp.resize(length + 1);
            const auto status = std::vsnprintf(temp.data(), temp.size(), format, args);
            if (status < 0)
                throw std::runtime_error {"string formatting error"};
            length = static_cast<std::size_t>(status);
        }

        __ss__<<temp.data()<<"\n";
        printf("%s", __ss__.str().c_str());
        pthread_mutex_unlock(&gCommonLogMutex);
#endif
    }
}