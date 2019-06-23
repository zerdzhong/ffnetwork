set(SOURCE_FILES 
    Client.cpp
    Request.cpp
    RequestImpl.h
    RequestImpl.cpp
    RequestTaskImpl.h
    RequestTaskImpl.cpp
)

add_library(ffnetwork 
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/Client.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/RequestTask.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/RequestTaskDelegate.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/Request.h"
    "${FFNETWORK_INCLUDE_DIRECTORY}/ffnetwork/Response.h"
    ${SOURCE_FILES}
)

target_include_directories(ffnetwork PUBLIC "${FFNETWORK_INCLUDE_DIRECTORY}")