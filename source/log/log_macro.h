#ifndef FFNETWORK_LOG_MACRO_H
#define FFNETWORK_LOG_MACRO_H

#include "logger.h"

#undef LOG_TAG
#define LOG_TAG "ffnetwork"

#ifndef MODULE_TAG
#define MODULE_TAG "NoTag"
#endif

#define LLOG(V,...) do { \
    ffnetwork::log(ffnetwork::ELOGLEVEL_##V, LOG_TAG, MODULE_TAG, __VA_ARGS__); \
}while(0)

#define LOGA(...)          LLOG(INFO,__VA_ARGS__)
#define LOGI(...)          LLOG(INFO,__VA_ARGS__)
#define LOGD(...)          LLOG(DEBUG,__VA_ARGS__)
#define LOGW(...)          LLOG(DEBUG,__VA_ARGS__)
#define LOGE(...)          LLOG(ERROR,__VA_ARGS__)

#endif //FFNETWORK_LOG_MACRO_H