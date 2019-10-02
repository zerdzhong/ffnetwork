#include <algorithm>
#include <iostream>
#include "logging.h"

#if defined(ANDROID)
#include <android/log.h>
#elif defined(DARWIN)
#include <syslog.h>
#endif

namespace ffbase {

const char* const kLogLevelNames[LOG_NUM_LEVELS] = {"INFO",
    "WARNING",
    "ERROR",
    "FATAL"
};

const char* GetNameForLogLevel(LogLevel level) {
    if (level >= LOG_INFO && level < LOG_NUM_LEVELS) {
        return kLogLevelNames[level];
    }
    
    return "UNKNOWN";
}

const char* StripDots(const char* path) {
  while (strncmp(path, "../", 3) == 0)
    path += 3;
  return path;
}

const char* StripPath(const char* path) {
  auto* p = strrchr(path, '/');
  if (p)
    return p + 1;
  else
    return path;
}

#pragma mark- log_message

LogMessage::LogMessage(LogLevel level, const char* file, int line, const char* condition)
:level_(level), file_(file), line_(line)
{
    stream_ << "[";
    if (level >= LOG_INFO) {
        stream_ << GetNameForLogLevel(level);
    }
    
    stream_ << ":" << (level > LOG_INFO ? StripDots(file_) : StripPath(file_))
    << "(" << line_ << ")] ";
    
    if (condition) {
        stream_ << "Check failed:" << condition << ".";
    }
}

LogMessage::~LogMessage() {
  stream_ << std::endl;

#if defined(ANDROID)
  android_LogPriority priority =
      (level_ < 0) ? ANDROID_LOG_VERBOSE : ANDROID_LOG_UNKNOWN;
  switch (level_) {
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
  __android_log_write(priority, "flutter", stream_.str().c_str());
#elif defined(DARWIN)
  syslog(LOG_ALERT, "%s", stream_.str().c_str());
#else
  std::cerr << stream_.str();
  std::cerr.flush();
#endif

  if (level_ >= LOG_FATAL) {
    abort();
  }
}

#pragma mark- log_setting

LogSettings g_log_settings;

void SetLogSettings(const LogSettings& settings) {
  // Validate the new settings as we set them.
  g_log_settings.min_log_level = std::min(LOG_FATAL, settings.min_log_level);
}

LogSettings GetLogSettings() {
  return g_log_settings;
}

int GetMinLogLevel() {
  return std::min(g_log_settings.min_log_level, LOG_FATAL);
}

int GetVlogVerbosity() {
  return std::max(-1, LOG_INFO - GetMinLogLevel());
}

bool ShouldCreateLogMessage(LogLevel level) {
  return level >= GetMinLogLevel();
}

}
