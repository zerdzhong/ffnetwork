#include "logging.h"
#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <iostream>

#if defined(ANDROID)
#include <android/log.h>
#elif (defined(MAC) || defined(IOS))
#include <os/log.h>
#include <syslog.h>
#endif

namespace ffbase {

#if defined(ANDROID)

void android_os_log(const LogLevel level, const char *log_inf) {
  android_LogPriority priority =
      (level < 0) ? ANDROID_LOG_VERBOSE : ANDROID_LOG_UNKNOWN;
  switch (level) {
  case LOG_INFO:
    priority = ANDROID_LOG_INFO;
    break;
  case LOG_WARNING:
    priority = ANDROID_LOG_WARN;
    break;
  case LOG_ERROR:
    priority = ANDROID_LOG_ERROR;
    break;
  case LOG_FATAL:
    priority = ANDROID_LOG_FATAL;
    break;
  }
  __android_log_write(priority, "ffbase", stream_.str().c_str());
}

#elif (defined(MAC) || defined(IOS))

#define FFBASE_DARWIN_LOG_HANDLE ffbase_darwin_log()

static os_log_t ffbase_darwin_log() {
  static os_log_t log = nullptr;

  if (log == nullptr) {
    log = os_log_create("com.zdzhong.ffbase", "ffbase");
  }

  return log;
}

void darwin_os_log(const LogLevel level, const char *log_info) {
  switch (level) {
  case LogLevel::INFO :
    os_log_info(FFBASE_DARWIN_LOG_HANDLE, "%s", log_info);
    break;
  case LogLevel::DEBUG:
  case LogLevel::WARNING :
    os_log_debug(FFBASE_DARWIN_LOG_HANDLE, "%s", log_info);
    break;
  case LogLevel::ERROR:
    os_log_error(FFBASE_DARWIN_LOG_HANDLE, "%s", log_info);
    break;
  case LogLevel::FATAL:
    os_log_fault(FFBASE_DARWIN_LOG_HANDLE, "%s", log_info);
    break;
  default:
    break;
  }
}

#endif

const char *const kLogLevelNames[NUM_LEVELS] = {"INFO", "DEBUG", "WARNING",
                                                    "ERROR", "FATAL"};

const char *GetNameForLogLevel(LogLevel level) {
  if (level >= 0 && level < NUM_LEVELS) {
    return kLogLevelNames[level];
  }

  return "UNKNOWN";
}

const char *StripDots(const char *path) {
  while (strncmp(path, "../", 3) == 0)
    path += 3;
  return path;
}

const char *StripPath(const char *path) {
  auto *p = strrchr(path, '/');
  if (p)
    return p + 1;
  else
    return path;
}

#pragma mark - log_message

LogMessage::LogMessage(LogLevel level, const char *file, int line,
                       const char *condition)
    : level_(level), file_(file), line_(line) {
  stream_ << "[";
  if (level >= 0) {
    stream_ << GetNameForLogLevel(level);
  }

  stream_ << ":" << StripPath(file_)
          << "(" << line_ << ")] ";

  if (condition) {
    stream_ << "Check failed:" << condition << ".";
  }
}

LogMessage::~LogMessage() {
  stream_ << std::endl;

#if defined(ANDROID)
  android_os_log(level_, stream_.str().c_str());
#elif (defined(MAC) || defined(IOS))
//  darwin_os_log(level_, stream_.str().c_str());
  std::cout << stream_.str();
  std::cout.flush();
#else
  std::cerr << stream_.str();
  std::cerr.flush();
#endif

  if (level_ >= FATAL) {
    abort();
  }
}

void LogMessage::print_log(const char *format, ...) {
  va_list args;
  va_start(args, format);

  char buf[1024] = {0};
  vsnprintf(buf, 1023, format, args);

  stream_ << buf;

  va_end(args);
}

#pragma mark - log_setting

LogSettings g_log_settings;

void SetLogSettings(const LogSettings &settings) {
  // Validate the new settings as we set them.
  g_log_settings.min_log_level = std::min(FATAL, settings.min_log_level);
}

LogSettings GetLogSettings() { return g_log_settings; }

int GetMinLogLevel() {
  return std::min(g_log_settings.min_log_level, FATAL);
}

bool ShouldCreateLogMessage(LogLevel level) {
  return level >= GetMinLogLevel();
}

} // namespace ffbase
