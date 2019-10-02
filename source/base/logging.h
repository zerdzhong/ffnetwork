#ifndef FFBASE_LOGGING_H
#define FFBASE_LOGGING_H

#include <sstream>
#include "macros.h"

namespace ffbase {

typedef int LogLevel;

// Default log levels. Negative values can be used for verbose log levels.
constexpr LogLevel LOG_INFO = 0;
constexpr LogLevel LOG_WARNING = 1;
constexpr LogLevel LOG_ERROR = 2;
constexpr LogLevel LOG_FATAL = 3;
constexpr LogLevel LOG_NUM_LEVELS = 4;

#ifdef DEBUG
const LogLevel LOG_DFATAL = LOG_ERROR;
#else
const LogLevel LOG_DFATAL = LOG_FATAL;
#endif

class LogMessageVoidify {
public:
    void operator&(std::ostream&) {}
};

class LogMessage {
public:
    LogMessage(LogLevel level, const char* file, int line, const char* condition);
    ~LogMessage();
    
    std::ostream& stream() { return stream_; }
    
private:
    std::ostringstream stream_;
    const LogLevel level_;
    const char* file_;
    const char* condition_;
    int line_;
    
    FF_DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

int GetVlogVerbosity();
bool ShouldCreateLogMessage(LogLevel level);

struct LogSettings {
    LogLevel min_log_level = LOG_INFO;
};

void SetLogSettings(const LogSettings& settings);
LogSettings GetLogSettings();
int GetMinLogLevel();

}// namespace ffbase

#define FF_LOG_STREAM(level) \
    ::ffbase::LogMessage(::fml::LOG_##level, __FILE__, __LINE__, nullptr).stream()

#define FF_LAZY_STREAM(stream, condition) \
    !(condition) ? (void)0 : ::ffbase::LogMessageVoidify() & (stream)

#define FF_EAT_STREAM_PARAMETERS(ignored) \
    (true || (ignored)) ? (void)0 : ::ffbase::LogMessageVoidify() &  \
    ::ffbase::LogMessage(::fml::LOG_FATAL, 0, 0, nullptr).stream()

#define FF_LOG_IS_ON(level) \
    (::ffbase::ShouldCreateLogMessage(::ffbase::LOG_##level))

#define FF_LOG(level) \
    FF_LAZY_STREAM(FF_LOG_STREAM(level), FF_LOG_IS_ON(level))

#define FF_CHECK(condition) \
    FF_LAZY_STREAM(::ffbase::LogMessage(::ffbase::LOG_FATAL, __FILE__, __LINE__, #condition) \
    .stream(), !(condition))

#ifndef DEBUG
    #define FF_DLOG(severity) FF_LOG(severity)
    #define FF_DCHECK(condition) FF_CHECK(condition)
#else
    #define FF_DLOG(severity) FF_EAT_STREAM_PARAMETERS(true)
    #define FF_DCHECK(condition) FF_EAT_STREAM_PARAMETERS(condition)
#endif

#define FF_NOTREACHED() FF_DCHECK(false)

#define FF_NOTIMPLEMENTED() \
    FF_LOG(ERROR) << "Not implemented in: " << __PRETTY_FUNCTION__


#endif //end FFBASE_LOGGING_H
